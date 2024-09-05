// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/AGX_LidarOutputBase.h"
#include "Sensors/AGX_RayPatternCustom.h"
#include "Sensors/AGX_RayPatternHorizontalSweep.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SensorUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"


#define LOCTEXT_NAMESPACE "AGX_LidarSensor"

UAGX_LidarSensorComponent::UAGX_LidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetModel(EAGX_LidarModel::GenericHorizontalSweep);

	static const TCHAR* DefaultNiagaraSystem =
		TEXT("StaticMesh'/AGXUnreal/Sensor/Lidar/NS_LidarNiagaraSystem.NS_LidarNiagaraSystem'");
	NiagaraSystemAsset =
		FAGX_ObjectUtilities::GetAssetFromPath<UNiagaraSystem>(DefaultNiagaraSystem);
}

void UAGX_LidarSensorComponent::SetModel(EAGX_LidarModel InModel)
{
	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Set Model was called during Play on Lidar Sensor '%s' in '%s'. This function "
					 "may only be called before Play."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return;
	}

	Range = FAGX_SensorUtilities::GetRangeFrom(InModel);
	BeamDivergence = FAGX_SensorUtilities::GetBeamDivergenceFrom(InModel);
	BeamExitRadius = FAGX_SensorUtilities::GetBeamExitRadiusFrom(InModel);
	RayPattern = FAGX_SensorUtilities::GetRayPatternFrom(InModel);
	bEnableDistanceGaussianNoise =
		FAGX_SensorUtilities::GetEnableDistanceGaussianNoiseFrom(InModel);
	DistanceNoiseSettings = FAGX_SensorUtilities::GetDistanceGaussianNoiseFrom(InModel);
}

EAGX_LidarModel UAGX_LidarSensorComponent::GetModel() const
{
	return Model;
}

void UAGX_LidarSensorComponent::SetRange(FAGX_RealInterval InRange)
{
	Range = InRange;

	if (HasNative())
		NativeBarrier.SetRange(InRange);
}

FAGX_RealInterval UAGX_LidarSensorComponent::GetRange() const
{
	if (HasNative())
		return NativeBarrier.GetRange();

	return Range;
}

void UAGX_LidarSensorComponent::SetBeamDivergence(double InBeamDivergence)
{
	BeamDivergence = InBeamDivergence;

	if (HasNative())
		NativeBarrier.SetBeamDivergence(InBeamDivergence);
}

double UAGX_LidarSensorComponent::GetBeamDivergence() const
{
	if (HasNative())
		return NativeBarrier.GetBeamDivergence();

	return BeamDivergence;
}

void UAGX_LidarSensorComponent::SetBeamExitRadius(double InBeamExitRadius)
{
	BeamExitRadius = InBeamExitRadius;

	if (HasNative())
		NativeBarrier.SetBeamExitRadius(InBeamExitRadius);
}

double UAGX_LidarSensorComponent::GetBeamExitRadius() const
{
	if (HasNative())
		return NativeBarrier.GetBeamExitRadius();

	return BeamExitRadius;
}

void UAGX_LidarSensorComponent::SetEnableRemovePointsMisses(bool bEnable)
{
	bEnableRemovePointsMisses = bEnable;

	if (HasNative())
		NativeBarrier.SetEnableRemoveRayMisses(bEnable);
}

bool UAGX_LidarSensorComponent::GetEnableRemovePointsMisses() const
{
	if (HasNative())
		return NativeBarrier.GetEnableRemoveRayMisses();

	return bEnableRemovePointsMisses;
}

void UAGX_LidarSensorComponent::SetEnableDistanceGaussianNoise(bool bEnable)
{
	bEnableDistanceGaussianNoise = bEnable;

	if (HasNative())
	{
		if (bEnable)
		{
			NativeBarrier.EnableDistanceGaussianNoise(
				DistanceNoiseSettings.Mean, DistanceNoiseSettings.StandardDeviation,
				DistanceNoiseSettings.StandardDeviationSlope);
		}
		else
		{
			NativeBarrier.DisableDistanceGaussianNoise();
		}
	}
}

UNiagaraComponent* UAGX_LidarSensorComponent::GetSpawnedNiagaraSystemComponent()
{
	return NiagaraSystemComponent;
}

void UAGX_LidarSensorComponent::Step()
{
	if (HasNative())
		NativeBarrier.SetTransform(GetComponentTransform());
}

bool UAGX_LidarSensorComponent::AddOutput(FAGX_LidarOutputBase& InOutput)
{
	auto Native = GetOrCreateNative();
	if (Native == nullptr)
		return false;

	Native->AddOutput(*InOutput.GetOrCreateNative());
	return true;
}

bool UAGX_LidarSensorComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarBarrier* UAGX_LidarSensorComponent::GetOrCreateNative()
{
	if (HasNative())
		return GetNative();

	if (RayPattern == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("No Ray Pattern selected for Lidar Sensor '%s' in '%s'. Make sure a valid "
					 "Ray Pattern has been selected."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return nullptr;
	}

	if (Cast<UAGX_RayPatternCustom>(RayPattern) != nullptr)
	{
		PatternFetcher.SetLidar(this);
		NativeBarrier.AllocateNativeRayPatternCustom(&PatternFetcher);
		AGX_CHECK(NativeBarrier.HasNative());
	}
	else if (auto Pattern = Cast<UAGX_RayPatternHorizontalSweep>(RayPattern))
	{
		NativeBarrier.AllocateNativeLidarRayPatternHorizontalSweep(
			Pattern->FOV, Pattern->Resolution, Pattern->Frequency);
		AGX_CHECK(NativeBarrier.HasNative());
	}
	else
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT(
					"Unknown Ray Pattern selected for Lidar Sensor '%s' in '%s'. Make sure a valid "
					"Ray Pattern has been selected."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return nullptr;
	}

	AGX_CHECK(NativeBarrier.HasNative());
	UpdateNativeProperties();
	return GetNative();
}

FLidarBarrier* UAGX_LidarSensorComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FLidarBarrier* UAGX_LidarSensorComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_LidarSensorComponent::CopyFrom(const UAGX_LidarSensorComponent& Source)
{
	Range = Source.Range;
	BeamDivergence = Source.BeamDivergence;
	BeamExitRadius = Source.BeamExitRadius;
	RayPattern = Source.RayPattern;
	bEnableDistanceGaussianNoise = Source.bEnableDistanceGaussianNoise;
	DistanceNoiseSettings = Source.DistanceNoiseSettings;
}

#if WITH_EDITOR
void UAGX_LidarSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableRendering && NiagaraSystemAsset != nullptr)
	{
		NiagaraSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystemAsset, this, NAME_None, FVector::ZeroVector,
			FRotator::ZeroRotator, FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset,
			false, ENCPoolMethod::None);
	}
}

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
			GET_MEMBER_NAME_CHECKED(ThisClass, Model),
			GET_MEMBER_NAME_CHECKED(ThisClass, RayPattern),
			GET_MEMBER_NAME_CHECKED(ThisClass, bEnableRendering),
			GET_MEMBER_NAME_CHECKED(ThisClass, NiagaraSystemAsset)
		};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}

	return SuperCanEditChange;
}

void UAGX_LidarSensorComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_LidarSensorComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_LidarSensorComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER(Range);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BeamDivergence);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BeamExitRadius);

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnableRemovePointsMisses),
		[](ThisClass* This)
		{ This->SetEnableRemovePointsMisses(This->bEnableRemovePointsMisses); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnableDistanceGaussianNoise),
		[](ThisClass* This)
		{ This->SetEnableDistanceGaussianNoise(This->bEnableDistanceGaussianNoise); });
}
#endif // WITH_EDITOR

void UAGX_LidarSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetRange(Range);
	NativeBarrier.SetBeamDivergence(BeamDivergence);
	NativeBarrier.SetBeamExitRadius(BeamExitRadius);
	NativeBarrier.SetEnableRemoveRayMisses(bEnableRemovePointsMisses);

	SetEnableDistanceGaussianNoise(bEnableDistanceGaussianNoise);
}

TArray<FTransform> UAGX_LidarSensorComponent::FetchRayTransforms()
{
	AGX_CHECK(Cast<UAGX_RayPatternCustom>(RayPattern) != nullptr);
	if (!OnFetchRayTransforms.IsBound())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Lidar Sensor '%s' in '%s' uses Custom Scan Pattern but the "
				 "OnFetchRayTransforms delegate has not been assinged. Assign the "
				 "OnFetchRayTransforms delegate in order to use a Custom scan pattern."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return TArray<FTransform>();
	}

	return OnFetchRayTransforms.Execute();
}

FAGX_CustomPatternInterval UAGX_LidarSensorComponent::FetchNextInterval()
{
	AGX_CHECK(Cast<UAGX_RayPatternCustom>(RayPattern) != nullptr);
	if (!OnFetchNextPatternInterval.IsBound())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Lidar Sensor '%s' in '%s' uses Custom Scan Pattern but the "
				 "FOnFetchNextPatternInterval delegate has not been assinged. Assign the "
				 "FOnFetchNextPatternInterval delegate in order to use a Custom scan pattern."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return FAGX_CustomPatternInterval();
	}

	double TimeStamp = 0.0;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		TimeStamp = Sim->GetTimeStamp();

	return OnFetchNextPatternInterval.Execute(TimeStamp);
}

#undef LOCTEXT_NAMESPACE
