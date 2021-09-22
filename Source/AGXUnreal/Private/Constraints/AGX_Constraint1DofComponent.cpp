#include "Constraints/AGX_Constraint1DofComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_UpropertyDispatcher.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

UAGX_Constraint1DofComponent::UAGX_Constraint1DofComponent()
{
}

UAGX_Constraint1DofComponent::UAGX_Constraint1DofComponent(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraintRotational,
	bool bInIsLockControllerEditable)
	: UAGX_ConstraintComponent(LockedDofsOrdered)
	, ElectricMotorController(bIsSecondaryConstraintRotational)
	, FrictionController(bIsSecondaryConstraintRotational)
	, LockController(bIsSecondaryConstraintRotational)
	, RangeController(bIsSecondaryConstraintRotational)
	, TargetSpeedController(bIsSecondaryConstraintRotational)
	, bIsLockControllerEditable(bInIsLockControllerEditable)
{
}

UAGX_Constraint1DofComponent::~UAGX_Constraint1DofComponent()
{
}

namespace
{
	FConstraint1DOFBarrier* Get1DOFBarrier(UAGX_Constraint1DofComponent& Constraint)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FConstraint1DOFBarrier*>(Constraint.GetNative());
	}

	const FConstraint1DOFBarrier* Get1DOFBarrier(const UAGX_Constraint1DofComponent& Constraint)
	{
		return static_cast<const FConstraint1DOFBarrier*>(Constraint.GetNative());
	}
}

#if WITH_EDITOR
void UAGX_Constraint1DofComponent::PostInitProperties()
{
	Super::PostInitProperties();

	FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks(
		PropertyDispatcher, &ElectricMotorController,
		GET_MEMBER_NAME_CHECKED(ThisClass, ElectricMotorController));

	FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks(
		PropertyDispatcher, &FrictionController,
		GET_MEMBER_NAME_CHECKED(ThisClass, FrictionController));

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, &LockController, GET_MEMBER_NAME_CHECKED(ThisClass, LockController));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, &RangeController, GET_MEMBER_NAME_CHECKED(ThisClass, RangeController));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, &TargetSpeedController,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetSpeedController));
}

void UAGX_Constraint1DofComponent::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	PropertyDispatcher.Trigger(PropertyChangedEvent, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_Constraint1DofComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.Num() > 2)
	{
		// The cases fewer chain elements are handled by PostEditChangeProperty, which is called by
		// UObject's PostEditChangeChainProperty.
		PropertyDispatcher.Trigger(PropertyChangedEvent, this);
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

float UAGX_Constraint1DofComponent::GetAngle() const
{
	return Get1DOFBarrier(*this)->GetAngle();
}

namespace AGX_Constraint1DofComponent_helpers
{
	void InitializeControllerBarriers(UAGX_Constraint1DofComponent& Constraint)
	{
		FConstraint1DOFBarrier* Barrier = Get1DOFBarrier(Constraint);
		Constraint.ElectricMotorController.InitializeBarrier(Barrier->GetElectricMotorController());
		Constraint.FrictionController.InitializeBarrier(Barrier->GetFrictionController());
		Constraint.LockController.InitializeBarrier(Barrier->GetLockController());
		Constraint.RangeController.InitializeBarrier(Barrier->GetRangeController());
		Constraint.TargetSpeedController.InitializeBarrier(Barrier->GetTargetSpeedController());
	}
}

void UAGX_Constraint1DofComponent::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint1DofComponent_helpers::InitializeControllerBarriers(*this);
}

void UAGX_Constraint1DofComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX Constraint '%s' is trying to update native properties while not having a "
				 "native handle."),
			*GetName());
		return;
	}

	Super::UpdateNativeProperties();

	ElectricMotorController.UpdateNativeProperties();
	FrictionController.UpdateNativeProperties();
	LockController.UpdateNativeProperties();
	RangeController.UpdateNativeProperties();
	TargetSpeedController.UpdateNativeProperties();
}

void UAGX_Constraint1DofComponent::SetNativeAddress(uint64 NativeAddress)
{
	Super::SetNativeAddress(NativeAddress);
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint1DofComponent_helpers::InitializeControllerBarriers(*this);
}
