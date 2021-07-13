#include "Wire/AGX_WireWinchComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_UpropertyDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_NativeOwnerInstanceData.h"

FVector UAGX_WireWinchComponent::ComputeBodyRelativeLocation()
{
	FVector WorldLocation = GetComponentTransform().TransformPosition(WireWinch.Location);
	if (UAGX_RigidBodyComponent* Body = WireWinch.GetBodyAttachment())
	{
		return Body->GetComponentTransform().InverseTransformPosition(WorldLocation);
	}
	else
	{
		return WorldLocation;
	}
}

FRotator UAGX_WireWinchComponent::ComputeBodyRelativeRotation()
{
	FQuat WorldRotation =
		GetComponentTransform().TransformRotation(WireWinch.Rotation.Quaternion());

	if (UAGX_RigidBodyComponent* Body = WireWinch.GetBodyAttachment())
	{
		return Body->GetComponentTransform().InverseTransformRotation(WorldRotation).Rotator();
	}
	else
	{
		return WorldRotation.Rotator();
	}
}

bool UAGX_WireWinchComponent::HasNative() const
{
	return WireWinch.HasNative();
}

uint64 UAGX_WireWinchComponent::GetNativeAddress() const
{
	return static_cast<uint64>(WireWinch.GetNativeAddress());
}

void UAGX_WireWinchComponent::AssignNative(uint64 NativeAddress)
{
	check(!HasNative());
	WireWinch.AssignNative(static_cast<uintptr_t>(NativeAddress));
}

TStructOnScope<FActorComponentInstanceData> UAGX_WireWinchComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this, [](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_WireWinchComponent::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, PulledInLength),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetPulledInLength(Winch->WireWinch.PulledInLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bMotorEnabled),
		[](ThisClass* Winch) { Winch->WireWinch.SetMotorEnabled(Winch->WireWinch.bMotorEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, TargetSpeed),
		[](ThisClass* Winch) { Winch->WireWinch.SetTargetSpeed(Winch->WireWinch.TargetSpeed); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, MotorForceRange),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetMotorForceRange(Winch->WireWinch.MotorForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bBrakeEnabled),
		[](ThisClass* Winch) { Winch->WireWinch.SetBrakeEnabled(Winch->WireWinch.bBrakeEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, BrakeForceRange),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetBrakeForceRange(Winch->WireWinch.BrakeForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bEmergencyBrakeEnabled),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetEmergencyBrakeEnabled(Winch->WireWinch.bEmergencyBrakeEnabled); });
#endif
}

#if WITH_EDITOR
void UAGX_WireWinchComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_WireWinchComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.Num() > 2)
	{
		// The cases fewer chain elements are handled by PostEditChangeProperty, which is called by
		// UObject's PostEditChangeChainProperty.
		FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent, this);
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif
