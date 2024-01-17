// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Async/ParallelFor.h"
#include "CollisionQueryParams.h"
#include "Containers/ArrayView.h"

// Standard library includes.
#include <algorithm>
#include <mutex>

UAGX_LidarSensorComponent::UAGX_LidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

namespace AGX_LidarSensorComponent_helpers
{
	void DrawDebugPoints(
		UWorld* World, const FTransform& LocalFrame, TArrayView<FAGX_LidarScanPoint> Points)
	{
		if (World == nullptr)
			return;

		for (const auto& P : Points)
		{
			const FVector PGlobal = LocalFrame.TransformPositionNoScale(P.Position);
			DrawDebugPoint(World, PGlobal, 5.f, FColor::Red, false, 0.06, 99);
		}
	}

	double ApproximateIntensity(
		const FHitResult& HitResult, const FVector_NetQuantizeNormal& Direction)
	{
		// Intensity based on angle of incident.
		double Intensity = std::max(0.0, -Direction.Dot(HitResult.Normal));

		// Take material Roughness into account.
		if (HitResult.Component != nullptr)
		{
			UMaterialInterface* MaterialInterf = HitResult.Component->GetMaterial(0);
			FMaterialParameterInfo Info;
			Info.Name = TEXT("Roughness");
			float Roughness;
			if (MaterialInterf->GetScalarParameterValue(Info, Roughness))
			{
				Intensity *= (1.0 - std::max(static_cast<double>(Roughness), 0.0));
			}
		}

		return Intensity;
	}

	struct LidarScanRequestParams
	{
		LidarScanRequestParams() = default;

		LidarScanRequestParams(
			const FTransform& InOrigin, const FVector2D& InFOV, const FVector2D& InResolution)
			: Origin(InOrigin)
			, FOV(InFOV)
			, Resolution(InResolution)
		{
		}

		const FTransform Origin {FTransform::Identity};
		const FVector2D FOV {FVector2D::ZeroVector};
		const FVector2D Resolution {FVector2D::ZeroVector};
		double TimeStamp {0.0};
		double FractionStart {0.0};
		double FractionEnd {0.0};
		double Range {0.0};
		bool bCalculateIntensity {true};
	};

	TArrayView<FAGX_LidarScanPoint> PerformPartialScanCPU(
		UWorld* World, const LidarScanRequestParams& Params, TArray<FAGX_LidarScanPoint>& OutData)
	{
		if (World == nullptr || Params.FractionEnd <= Params.FractionStart || Params.Range <= 0.0)
			return {};

		const int32 NumRaysCycleX = static_cast<int32>(Params.FOV.X / Params.Resolution.X);
		const int32 NumRaysCycleY = static_cast<int32>(Params.FOV.Y / Params.Resolution.Y);
		if (NumRaysCycleX <= 0 || NumRaysCycleY <= 0)
			return {};

		const int32 NumRaysCycle = NumRaysCycleX * NumRaysCycleY;
		const double NumRaysCycled = static_cast<double>(NumRaysCycle);
		const int32 FirstRay = [&Params, NumRaysCycled]()
		{
			int32 Start = std::max(static_cast<int32>(NumRaysCycled * Params.FractionStart), 0);

			// To avoid sampling the same point twice, we start one after the calculated start index
			// since it was scanned the previous round. If this is the first partial scan of a
			// cycle, then we start from the calculated start index (which should be zero).
			if (!FMath::IsNearlyZero(Params.FractionStart))
				Start += 1;

			return Start;
		}();

		const int32 LastRay =
			std::min(FMath::RoundToInt32(NumRaysCycled * Params.FractionEnd), NumRaysCycle - 1);

		const FVector StartGlobal = Params.Origin.GetLocation();
		const int32 NumPointsPreAppend = OutData.Num();
		const FCollisionQueryParams CollParams;

		// This number is somewhat arbitrary, but a good starting point.
		// The cost of starting several threads will at some point make single threaded run the
		// better option. Expected performance is on the order of hundreads of thousands of rays /
		// second on a single thread.
		static constexpr int32 MinRaysForMultithread = 300;
		const int32 NumRays = LastRay - FirstRay + 1;
		const bool RunMultithreaded =
			FPlatformProcess::SupportsMultithreading() && NumRays >= MinRaysForMultithread;
		const int32 NumThreads = FPlatformMisc::NumberOfWorkerThreadsToSpawn();
		const int32 NumRaysPerThread = RunMultithreaded ? NumRays / NumThreads : NumRays;
		std::mutex Mutex;
		ParallelFor(
			NumThreads,
			[&](int32 Thread)
			{
				const int32 ThreadFirstRay = FirstRay + Thread * NumRaysPerThread;
				int32 ThreadLastRayPlusOne = FirstRay + (Thread + 1) * NumRaysPerThread;
				if (Thread == NumThreads - 1) // Last thread.
					ThreadLastRayPlusOne = LastRay + 1;

				FHitResult HitResult;
				for (int32 Ray = ThreadFirstRay; Ray < ThreadLastRayPlusOne; Ray++)
				{
					const int32 IndexX = Ray % NumRaysCycleX;
					const int32 IndexY = Ray / NumRaysCycleX;
					const double AngRadX = FMath::DegreesToRadians(
						-Params.FOV.X / 2.0 + Params.Resolution.X * static_cast<double>(IndexX));
					const double AngRadY = FMath::DegreesToRadians(
						-Params.FOV.Y / 2.0 + Params.Resolution.Y * static_cast<double>(IndexY));

					// Dir is the normalized vector following the current laser ray. Here X is
					// horizontal and Y vertical.
					const FVector Dir(
						FMath::Sin(AngRadX) * FMath::Cos(AngRadY), FMath::Sin(AngRadY),
						FMath::Cos(AngRadX) * FMath::Cos(AngRadY));

					// Lidar sensors local coordinate system is x forwards, y to the right and z up.
					const FVector EndLocal(
						Dir.Z * Params.Range, Dir.X * Params.Range, -Dir.Y * Params.Range);
					const FVector EndGlobal = Params.Origin.TransformPositionNoScale(EndLocal);
					if (!World->LineTraceSingleByChannel(
							HitResult, StartGlobal, EndGlobal, ECC_Visibility, CollParams))
						continue;

					const FVector LocalPoint =
						Params.Origin.InverseTransformPositionNoScale(HitResult.Location);
					double Intensity = 0.0;
					if (Params.bCalculateIntensity)
					{
						const FVector_NetQuantizeNormal DirGlobal(
							(EndGlobal - StartGlobal).GetSafeNormal());
						Intensity = ApproximateIntensity(HitResult, DirGlobal);
					}

					std::lock_guard<std::mutex> Lock(Mutex);
					OutData.Add(FAGX_LidarScanPoint(
						FVector(LocalPoint.X, LocalPoint.Y, LocalPoint.Z), Params.TimeStamp,
						Intensity));
				}
			},
			!RunMultithreaded);

		const int32 NumNewPoints = OutData.Num() - NumPointsPreAppend;
		if (NumNewPoints == 0)
			return {};

		return MakeArrayView(&OutData[NumPointsPreAppend], NumNewPoints);
	}
}

void UAGX_LidarSensorComponent::RequestManualScan() const
{
	if (!bIsValid)
		return;

	if (ExecutionMode != EAGX_LidarExecutonMode::Manual)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("RequestManualScan was called on Lidar Sensor '%s' in Actor '%s' but the "
				 "ExecutionMode is not set to Manual. Doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	// Todo: implement.
}

void UAGX_LidarSensorComponent::BeginPlay()
{
	Super::BeginPlay();
	bIsValid = CheckValid();

	if (!bIsValid)
	{
		return;
	}

	if (ExecutionMode == EAGX_LidarExecutonMode::Auto)
	{
		// The Buffer can grow dynamically during Runtime, but this is a small optimization for the
		// first scan cycle.
		const double StoreFraction = ScanFrequency / OutputFrequency;
		const double NumPointsPerCycle = (FOV.X / Resolution.X) * (FOV.Y / Resolution.Y);
		const double SafetyMargin = 1.05;
		const int32 InitSize = static_cast<int32>(StoreFraction * NumPointsPerCycle * SafetyMargin);

		if (InitSize > 0)
			Buffer.Reserve(InitSize);
	}

	LidarState.ScanCycleDuration = 1.0 / ScanFrequency;
}

void UAGX_LidarSensorComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ExecutionMode != EAGX_LidarExecutonMode::Auto)
		return;

	if (!bIsValid)
		return;

	UpdateElapsedTime();

	if (SamplingType == EAGX_LidarSamplingType::CPU)
		ScanCPU();

	OutputPointCloudDataIfReady();
}

void UAGX_LidarSensorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	PointCloudDataOutput.Clear();
}

#if WITH_EDITOR
bool UAGX_LidarSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, ScanFrequency),
			GET_MEMBER_NAME_CHECKED(ThisClass, OutputFrequency),
			GET_MEMBER_NAME_CHECKED(ThisClass, SamplingType),
			GET_MEMBER_NAME_CHECKED(ThisClass, FOV /*clang-format padding*/),
			GET_MEMBER_NAME_CHECKED(ThisClass, Resolution)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}

	return SuperCanEditChange;
}
#endif

bool UAGX_LidarSensorComponent::CheckValid() const
{
	if (ScanFrequency <= 0.0 || OutputFrequency <= 0.0)
	{
		const FString Msg = FString::Printf(
			TEXT("Lidar Sensor '%s' in Actor '%s' has a non-positive Scan Frequency or Output "
				 "Frequency. Update these so that they are larger than zero."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (OutputFrequency < ScanFrequency)
	{
		const FString Msg = FString::Printf(
			TEXT("Lidar Sensor '%s' in Actor '%s' has an Output Frequency that is lower than the "
				 "Scan Frequency. Set the Output Frequency so that it is at least ar high as the "
				 "Scan Frequency."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	auto IsInRange = [](const FVector2D& V, double Min, double Max)
	{ return FMath::IsWithinInclusive(V.X, Min, Max) && FMath::IsWithinInclusive(V.Y, Min, Max); };

	if (!IsInRange(FOV, 0.0, 360.0) || !IsInRange(Resolution, 0.0, 180))
	{
		const FString Msg = FString::Printf(
			TEXT("Lidar Sensor '%s' in Actor '%s' has a FOV or Resolution outside of the valid "
				 "range. The x and y component of the FOV must both be between 0 and 360 and for "
				 "Resolution they must be between 0 and 180."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	return true;
}

void UAGX_LidarSensorComponent::UpdateElapsedTime()
{
	LidarState.ElapsedTimePrev = LidarState.ElapsedTime;
	LidarState.ElapsedTime = GetWorld()->GetTimeSeconds();
}

void UAGX_LidarSensorComponent::ScanCPU()
{
	using namespace AGX_LidarSensorComponent_helpers;
	AGX_CHECK(bIsValid);

	const double ScanCycleTimeElapsedPrev =
		LidarState.ElapsedTimePrev - LidarState.CurrentScanCycleStartTime;
	const double ScanCycleFractionPrev = ScanCycleTimeElapsedPrev / LidarState.ScanCycleDuration;
	AGX_CHECK(ScanCycleFractionPrev < 1.0);

	const double ScanCycleTimeElapsed =
		LidarState.ElapsedTime - LidarState.CurrentScanCycleStartTime;
	const double ScanCycleFraction = ScanCycleTimeElapsed / LidarState.ScanCycleDuration;

	AGX_CHECK(ScanCycleFraction > ScanCycleFractionPrev);

	LidarScanRequestParams Params(GetComponentTransform(), FOV, Resolution);
	{
		Params.TimeStamp = LidarState.ElapsedTime;
		Params.FractionStart = ScanCycleFractionPrev;
		Params.FractionEnd = std::min(ScanCycleFraction, 1.0);
		Params.Range = Range;
		Params.bCalculateIntensity = bCalculateIntensity;
	}

	auto NewPoints = PerformPartialScanCPU(GetWorld(), Params, Buffer);

	if (bDebugRenderPoints)
	{
		DrawDebugPoints(GetWorld(), GetComponentTransform(), NewPoints);
	}

	if (ScanCycleFraction >= 1.0)
	{
		// Set the state as to prepare the next cycle.
		LidarState.CurrentScanCycleStartTime = LidarState.ElapsedTime;
	}
}

void UAGX_LidarSensorComponent::OutputPointCloudDataIfReady()
{
	AGX_CHECK(bIsValid);

	const double OutputCycleTimeElapsed =
		LidarState.ElapsedTime - LidarState.CurrentOutputCycleStartTime;

	if (OutputCycleTimeElapsed >= LidarState.OutputCycleDuration)
	{
		PointCloudDataOutput.Broadcast(Buffer);
		Buffer.SetNum(0, false);
		LidarState.CurrentOutputCycleStartTime = LidarState.ElapsedTime;
	}
}
