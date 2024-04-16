// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_RayPatternCustom.h"
#include "Sensors/AGX_RayPatternHorizontalSweep.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_LidarSensorComponent::UAGX_LidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

void UAGX_LidarSensorComponent::SetBeamExitDiameter(double InBeamExitDiameter)
{
	BeamExitDiameter = InBeamExitDiameter;

	if (HasNative())
		NativeBarrier.SetBeamExitDiameter(InBeamExitDiameter);
}

double UAGX_LidarSensorComponent::GetBeamExitDiameter() const
{
	if (HasNative())
		return NativeBarrier.GetBeamExitDiameter();

	return BeamExitDiameter;
}

void UAGX_LidarSensorComponent::Step()
{
	if (HasNative())
		NativeBarrier.SetTransform(GetComponentTransform());
}

bool UAGX_LidarSensorComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

namespace AGX_LidarSensorComponent_helpers
{
	EAGX_LidarRayPattern GetTypeFrom(UAGX_RayPatternBase* Pattern)
	{
		if (Pattern == nullptr)
			return EAGX_LidarRayPattern::Invalid;

		if (Cast<UAGX_RayPatternHorizontalSweep>(Pattern) != nullptr)
			return EAGX_LidarRayPattern::HorizontalSweep;

		if (Cast<UAGX_RayPatternCustom>(Pattern) != nullptr)
			return EAGX_LidarRayPattern::Custom;

		UE_LOG(
			LogAGX, Error,
			TEXT("Unknown RayPattern type given to LidarSensorComponent::GetTypeFrom."));
		return EAGX_LidarRayPattern::Invalid;
	}
}

FLidarBarrier* UAGX_LidarSensorComponent::GetOrCreateNative()
{
	if (HasNative())
		return GetNative();

	EAGX_LidarRayPattern Pattern = AGX_LidarSensorComponent_helpers::GetTypeFrom(RayPattern);
	if (Pattern == EAGX_LidarRayPattern::Invalid)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Invalid Ray Pattern selected in Lidar Sensor '%s' in '%s'. Make sure a valid Ray "
			"Pattern has been selected.",
			SNotificationItem::CS_Fail);
		return nullptr;
	}

	if (Pattern == EAGX_LidarRayPattern::Custom)
		PatternFetcher.SetLidar(this);

	NativeBarrier.AllocateNative(Pattern, &PatternFetcher);
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

void UAGX_LidarSensorComponent::GetResultTest()
{
	NativeBarrier.GetResultTest(GetWorld(), GetComponentTransform());
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
			GET_MEMBER_NAME_CHECKED(ThisClass, RayPattern)};

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
	AGX_COMPONENT_DEFAULT_DISPATCHER(BeamExitDiameter);
}
#endif

void UAGX_LidarSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetRange(Range);
	NativeBarrier.SetBeamDivergence(BeamDivergence);
	NativeBarrier.SetBeamExitDiameter(BeamExitDiameter);
}

TArray<FTransform> UAGX_LidarSensorComponent::FetchRayTransforms()
{
	AGX_CHECK(
		AGX_LidarSensorComponent_helpers::GetTypeFrom(RayPattern) == EAGX_LidarRayPattern::Custom);
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
	AGX_CHECK(
		AGX_LidarSensorComponent_helpers::GetTypeFrom(RayPattern) == EAGX_LidarRayPattern::Custom);
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
