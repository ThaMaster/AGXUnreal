#include "Constraints/AGX_Constraint1DofComponent.h"

#include "AGX_LogCategory.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Constraint1DOFBarrier.h"

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

	void AddControllerPropertyCallbacks(
		FAGX_UpropertyDispatcher<UAGX_ConstraintComponent>& PropertyDispatcher, const FName& Member,
		FAGX_ConstraintController* Controller)
	{
		using Super = UAGX_Constraint1DofComponent::Super;
		using ThisClass = UAGX_Constraint1DofComponent;

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, bEnable),
			[Controller](Super*) { Controller->SetEnable(Controller->bEnable); });

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, Compliance),
			[Controller](Super*) { Controller->SetCompliance(Controller->Compliance); });

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, Damping),
			[Controller](Super*) { Controller->SetDamping(Controller->Damping); });

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, ForceRange),
			[Controller](Super*) { Controller->SetForceRange(Controller->ForceRange); });
	}

	void AddElectricMotorControllerPropertyCallbacks(
		FAGX_UpropertyDispatcher<UAGX_ConstraintComponent>& PropertyDispatcher,
		FAGX_ConstraintElectricMotorController* Controller)
	{
		FName Member =
			GET_MEMBER_NAME_CHECKED(UAGX_Constraint1DofComponent, ElectricMotorController);

		AddControllerPropertyCallbacks(PropertyDispatcher, Member, Controller);

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintElectricMotorController, Voltage),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetVoltage(Controller->Voltage);
			});

		PropertyDispatcher.Add(
			Member,
			GET_MEMBER_NAME_CHECKED(FAGX_ConstraintElectricMotorController, ArmatureResistance),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetArmatureRestistance(Controller->ArmatureResistance);
			});

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintElectricMotorController, TorqueConstant),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetTorqueConstant(Controller->TorqueConstant);
			});
	}

	void AddFrictionControllerPropertyCallbacks(
		FAGX_UpropertyDispatcher<UAGX_ConstraintComponent>& PropertyDispatcher,
		FAGX_ConstraintFrictionController* Controller)
	{
		FName Member = GET_MEMBER_NAME_CHECKED(UAGX_Constraint1DofComponent, FrictionController);

		AddControllerPropertyCallbacks(PropertyDispatcher, Member, Controller);

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintFrictionController, FrictionCoefficient),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetFrictionCoefficient(Controller->FrictionCoefficient);
			});

		PropertyDispatcher.Add(
			Member,
			GET_MEMBER_NAME_CHECKED(
				FAGX_ConstraintFrictionController, bEnableNonLinearDirectSolveUpdate),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetEnableNonLinearDirectSolveUpdate(
					Controller->bEnableNonLinearDirectSolveUpdate);
			});
	}

	void AddLockControllerPropertyCallbacks(
		FAGX_UpropertyDispatcher<UAGX_ConstraintComponent>& PropertyDispatcher,
		FAGX_ConstraintLockController* Controller)
	{
		FName Member = GET_MEMBER_NAME_CHECKED(UAGX_Constraint1DofComponent, LockController);

		AddControllerPropertyCallbacks(PropertyDispatcher, Member, Controller);

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintLockController, Position),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetPosition(Controller->Position);
			});
	}

	void AddRangeControllerPropertyCallbacks(
		FAGX_UpropertyDispatcher<UAGX_ConstraintComponent>& PropertyDispatcher,
		FAGX_ConstraintRangeController* Controller)
	{
		FName Member = GET_MEMBER_NAME_CHECKED(UAGX_Constraint1DofComponent, RangeController);

		AddControllerPropertyCallbacks(PropertyDispatcher, Member, Controller);

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangeController, Range),
			[Controller](UAGX_ConstraintComponent*) { Controller->SetRange(Controller->Range); });
	}

	void AddTargetSpeedControllerPropertyCallbacks(
		FAGX_UpropertyDispatcher<UAGX_ConstraintComponent>& PropertyDispatcher,
		FAGX_ConstraintTargetSpeedController* Controller)
	{
		FName Member = GET_MEMBER_NAME_CHECKED(UAGX_Constraint1DofComponent, TargetSpeedController);

		AddControllerPropertyCallbacks(PropertyDispatcher, Member, Controller);

		PropertyDispatcher.Add(
			Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintTargetSpeedController, Speed),
			[Controller](UAGX_ConstraintComponent*) { Controller->SetSpeed(Controller->Speed); });

		PropertyDispatcher.Add(
			Member,
			GET_MEMBER_NAME_CHECKED(FAGX_ConstraintTargetSpeedController, bLockedAtZeroSpeed),
			[Controller](UAGX_ConstraintComponent*) {
				Controller->SetLockedAtZeroSpeed(Controller->bLockedAtZeroSpeed);
			});
	}

}

void UAGX_Constraint1DofComponent::PostLoad()
{
	Super::PostLoad();
	AddElectricMotorControllerPropertyCallbacks(PropertyDispatcher, &ElectricMotorController);
	AddFrictionControllerPropertyCallbacks(PropertyDispatcher, &FrictionController);
	AddLockControllerPropertyCallbacks(PropertyDispatcher, &LockController);
	AddRangeControllerPropertyCallbacks(PropertyDispatcher, &RangeController);
	AddTargetSpeedControllerPropertyCallbacks(PropertyDispatcher, &TargetSpeedController);
}

float UAGX_Constraint1DofComponent::GetAngle() const
{
	return Get1DOFBarrier(*this)->GetAngle();
}

void UAGX_Constraint1DofComponent::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	FConstraint1DOFBarrier* Barrier = Get1DOFBarrier(*this);
	ElectricMotorController.InitializeBarrier(Barrier->GetElectricMotorController());
	FrictionController.InitializeBarrier(Barrier->GetFrictionController());
	LockController.InitializeBarrier(Barrier->GetLockController());
	RangeController.InitializeBarrier(Barrier->GetRangeController());
	TargetSpeedController.InitializeBarrier(Barrier->GetTargetSpeedController());
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
