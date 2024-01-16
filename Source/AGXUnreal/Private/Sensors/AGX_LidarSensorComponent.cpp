// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Standard library includes.
#include <algorithm>

UAGX_LidarSensorComponent::UAGX_LidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

namespace AGX_LidarSensorComponent_helpers
{
	void PerformPartialScanCPU(double FractionStart, double FractionEnd)
	{
		if (FractionEnd <= FractionStart)
			return;

		UE_LOG(LogTemp, Warning, TEXT("%f %f"), FractionStart, FractionEnd);
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
	PerformPartialScanCPU(ScanCycleFractionPrev, std::min(ScanCycleFraction, 1.0));

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
