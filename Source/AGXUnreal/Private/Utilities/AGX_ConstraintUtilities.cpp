// Copyright 2023, Algoryx Simulation AB.

#include "Utilities/AGX_ConstraintUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_Constraint1DofComponent& Component, const FConstraint1DOFBarrier& Barrier,
	bool ForceOverwriteInstances)
{
	TArray<FAGX_ConstraintElectricMotorController*> EMCInstances;
	TArray<FAGX_ConstraintFrictionController*> FCInstances;
	TArray<FAGX_ConstraintLockController*> LCInstances;
	TArray<FAGX_ConstraintRangeController*> RCInstances;
	TArray<FAGX_ConstraintTargetSpeedController*> TSCInstances;

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			EMCInstances.Add(&Instance->ElectricMotorController);
			FCInstances.Add(&Instance->FrictionController);
			LCInstances.Add(&Instance->LockController);
			RCInstances.Add(&Instance->RangeController);
			TSCInstances.Add(&Instance->TargetSpeedController);
		}
	}

	StoreElectricMotorController(
		Barrier, Component.ElectricMotorController, EMCInstances, ForceOverwriteInstances);
	StoreFrictionController(
		Barrier, Component.FrictionController, FCInstances, ForceOverwriteInstances);
	StoreLockController(Barrier, Component.LockController, LCInstances, ForceOverwriteInstances);
	StoreRangeController(Barrier, Component.RangeController, RCInstances, ForceOverwriteInstances);
	StoreTargetSpeedController(
		Barrier, Component.TargetSpeedController, TSCInstances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_Constraint2DofComponent& Component, const FConstraint2DOFBarrier& Barrier,
	bool ForceOverwriteInstances)
{
	const EAGX_Constraint2DOFFreeDOF First = EAGX_Constraint2DOFFreeDOF::FIRST;
	const EAGX_Constraint2DOFFreeDOF Second = EAGX_Constraint2DOFFreeDOF::SECOND;
	TArray<FAGX_ConstraintElectricMotorController*> EMCInstances1;
	TArray<FAGX_ConstraintElectricMotorController*> EMCInstances2;
	TArray<FAGX_ConstraintFrictionController*> FCInstances1;
	TArray<FAGX_ConstraintFrictionController*> FCInstances2;
	TArray<FAGX_ConstraintLockController*> LCInstances1;
	TArray<FAGX_ConstraintLockController*> LCInstances2;
	TArray<FAGX_ConstraintRangeController*> RCInstances1;
	TArray<FAGX_ConstraintRangeController*> RCInstances2;
	TArray<FAGX_ConstraintTargetSpeedController*> TSCInstances1;
	TArray<FAGX_ConstraintTargetSpeedController*> TSCInstances2;

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			EMCInstances1.Add(&Instance->ElectricMotorController1);
			EMCInstances2.Add(&Instance->ElectricMotorController2);
			FCInstances1.Add(&Instance->FrictionController1);
			FCInstances2.Add(&Instance->FrictionController2);
			LCInstances1.Add(&Instance->LockController1);
			LCInstances2.Add(&Instance->LockController2);
			RCInstances1.Add(&Instance->RangeController1);
			RCInstances2.Add(&Instance->RangeController2);
			TSCInstances1.Add(&Instance->TargetSpeedController1);
			TSCInstances2.Add(&Instance->TargetSpeedController2);
		}
	}

	StoreElectricMotorController(
		Barrier, Component.ElectricMotorController1, First, EMCInstances1, ForceOverwriteInstances);
	StoreElectricMotorController(
		Barrier, Component.ElectricMotorController2, Second, EMCInstances2,
		ForceOverwriteInstances);

	StoreFrictionController(
		Barrier, Component.FrictionController1, First, FCInstances1, ForceOverwriteInstances);
	StoreFrictionController(
		Barrier, Component.FrictionController2, Second, FCInstances2, ForceOverwriteInstances);

	StoreLockController(
		Barrier, Component.LockController1, First, LCInstances1, ForceOverwriteInstances);
	StoreLockController(
		Barrier, Component.LockController2, Second, LCInstances2, ForceOverwriteInstances);

	StoreRangeController(
		Barrier, Component.RangeController1, First, RCInstances1, ForceOverwriteInstances);
	StoreRangeController(
		Barrier, Component.RangeController2, Second, RCInstances2, ForceOverwriteInstances);

	StoreTargetSpeedController(
		Barrier, Component.TargetSpeedController1, First, TSCInstances1, ForceOverwriteInstances);
	StoreTargetSpeedController(
		Barrier, Component.TargetSpeedController2, Second, TSCInstances2, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreElectricMotorController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller,
	TArray<FAGX_ConstraintElectricMotorController*>& Instances, bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetElectricMotorController(), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreElectricMotorController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof, TArray<FAGX_ConstraintElectricMotorController*>& Instances,
	bool ForceOverwriteInstances)
{
	Controller.CopyFrom(
		*Barrier.GetElectricMotorController(Dof), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreFrictionController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
	TArray<FAGX_ConstraintFrictionController*>& Instances, bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetFrictionController(), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreFrictionController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof, TArray<FAGX_ConstraintFrictionController*>& Instances,
	bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetFrictionController(Dof), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreLockController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
	TArray<FAGX_ConstraintLockController*>& Instances, bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetLockController(), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreLockController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof, TArray<FAGX_ConstraintLockController*>& Instances,
	bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetLockController(Dof), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreRangeController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
	TArray<FAGX_ConstraintRangeController*>& Instances, bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetRangeController(), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreRangeController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof, TArray<FAGX_ConstraintRangeController*>& Instances,
	bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetRangeController(Dof), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreTargetSpeedController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
	TArray<FAGX_ConstraintTargetSpeedController*>& Instances, bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetTargetSpeedController(), Instances, ForceOverwriteInstances);
}

void FAGX_ConstraintUtilities::StoreTargetSpeedController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof, TArray<FAGX_ConstraintTargetSpeedController*>& Instances,
	bool ForceOverwriteInstances)
{
	Controller.CopyFrom(*Barrier.GetTargetSpeedController(Dof), Instances, ForceOverwriteInstances);
}

#if WITH_EDITOR

/*
 * Helper functions for setting up Constraint Controller callbacks for Property
 * Changed events. This used to be a set of small and simple functions, but an
 * Unreal Engine bug related to editing multiple Components in an instance of a
 * Blueprint during a Play In Editor session made the old approach impossible. A
 * bug fix is planned for Unreal Engine 5.1 but until we are forced to use this
 * long version. See internal issue 675, 689, and UDN question
 * https://udn.unrealengine.com/s/case/5004z00001eEzehAAC/editing-multiple-components-in-a-blueprint-instance-only-calls-posteditchangechainproperty-on-one-of-them
 *
 * The bug is that PostEditChangeChainProperty isn't called on all modified
 * objects, which causes some AGX Dynamics native objects to not be updated with
 * the change. The result is that there is a mismatch between the Unreal Engine
 * state and the AGX Dynamics state for some of the selected Components. The
 * workaround is to update all selected objects every time any modification is
 * detected. A consequence of this is that we cannot allow object-specific data
 * in the callbacks, i.e. lambdas with captures are now forbidden. We used to
 * capture the Constraint Controller to update in the lambda, but now we must
 * pass a getter function instead, and we need lots of templating to handle all
 * the various different types that may be passed.
 *
 * It is possible that this approach has become so complicated that a there is
 * a simpler and better way to do it.
 */

template <typename UConstraintClass, typename FControllerClass>
void FAGX_ConstraintUtilities::AddControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FControllerClass*(UConstraintClass*)> GetController, const FName& Member)
{
	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, bEnable),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetEnable(GetController(EditedObject)->bEnable); });

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, Compliance),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetCompliance(GetController(EditedObject)->Compliance); });

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, SpookDamping),
		[GetController](UConstraintClass* EditedObject) {
			GetController(EditedObject)->SetSpookDamping(GetController(EditedObject)->SpookDamping);
		});

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintController, ForceRange),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetForceRange(GetController(EditedObject)->ForceRange); });
}

template AGXUNREAL_API void FAGX_ConstraintUtilities::AddControllerPropertyCallbacks<
	UAGX_Constraint1DofComponent, FAGX_ConstraintController>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint1DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintController*(UAGX_Constraint1DofComponent*)> GetController,
	const FName& Member);

template AGXUNREAL_API void FAGX_ConstraintUtilities::AddControllerPropertyCallbacks<
	UAGX_Constraint2DofComponent, FAGX_ConstraintController>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

template <typename UConstraintClass>
void FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FAGX_ConstraintElectricMotorController*(UConstraintClass*)> GetController,
	const FName& Member)
{
	AddControllerPropertyCallbacks(PropertyDispatcher, GetController, Member);

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintElectricMotorController, Voltage),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetVoltage(GetController(EditedObject)->Voltage); });

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintElectricMotorController, ArmatureResistance),
		[GetController](UConstraintClass* EditedObject)
		{
			GetController(EditedObject)
				->SetArmatureRestistance(GetController(EditedObject)->ArmatureResistance);
		});

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintElectricMotorController, TorqueConstant),
		[GetController](UConstraintClass* EditedObject) {
			GetController(EditedObject)
				->SetTorqueConstant(GetController(EditedObject)->TorqueConstant);
		});
}

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks<UAGX_Constraint1DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint1DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintElectricMotorController*(UAGX_Constraint1DofComponent*)> GetController,
	const FName& Member);

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks<UAGX_Constraint2DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintElectricMotorController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

template <typename UConstraintClass>
void FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FAGX_ConstraintFrictionController*(UConstraintClass*)> GetController,
	const FName& Member)
{
	AddControllerPropertyCallbacks(PropertyDispatcher, GetController, Member);

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintFrictionController, FrictionCoefficient),
		[GetController](UConstraintClass* EditedObject)
		{
			GetController(EditedObject)
				->SetFrictionCoefficient(GetController(EditedObject)->FrictionCoefficient);
		});

	PropertyDispatcher.Add(
		Member,
		GET_MEMBER_NAME_CHECKED(
			FAGX_ConstraintFrictionController, bEnableNonLinearDirectSolveUpdate),
		[GetController](UConstraintClass* EditedObject)
		{
			GetController(EditedObject)
				->SetEnableNonLinearDirectSolveUpdate(
					GetController(EditedObject)->bEnableNonLinearDirectSolveUpdate);
		});
}

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks<UAGX_Constraint1DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint1DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintFrictionController*(UAGX_Constraint1DofComponent*)> GetController,
	const FName& Member);

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks<UAGX_Constraint2DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintFrictionController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

template <typename UConstraintClass>
void FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FAGX_ConstraintLockController*(UConstraintClass*)> GetController, const FName& Member)
{
	AddControllerPropertyCallbacks(PropertyDispatcher, GetController, Member);

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintLockController, Position),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetPosition(GetController(EditedObject)->Position); });
}

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks<UAGX_Constraint1DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint1DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintLockController*(UAGX_Constraint1DofComponent*)> GetController,
	const FName& Member);

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks<UAGX_Constraint2DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintLockController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

template <typename UConstraintClass>
void FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FAGX_ConstraintRangeController*(UConstraintClass*)> GetController,
	const FName& Member)
{
	AddControllerPropertyCallbacks(PropertyDispatcher, GetController, Member);

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangeController, Range),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetRange(GetController(EditedObject)->Range); });
}

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks<UAGX_Constraint1DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint1DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintRangeController*(UAGX_Constraint1DofComponent*)> GetController,
	const FName& Member);

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks<UAGX_Constraint2DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintRangeController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

template <typename UConstraintClass>
void FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FAGX_ConstraintTargetSpeedController*(UConstraintClass*)> GetController,
	const FName& Member)
{
	AddControllerPropertyCallbacks(PropertyDispatcher, GetController, Member);

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintTargetSpeedController, Speed),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetSpeed(GetController(EditedObject)->Speed); });

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintTargetSpeedController, bLockedAtZeroSpeed),
		[GetController](UConstraintClass* EditedObject)
		{
			GetController(EditedObject)
				->SetLockedAtZeroSpeed(GetController(EditedObject)->bLockedAtZeroSpeed);
		});
}

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks<UAGX_Constraint1DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint1DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintTargetSpeedController*(UAGX_Constraint1DofComponent*)> GetController,
	const FName& Member);

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks<UAGX_Constraint2DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintTargetSpeedController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

template <typename UConstraintClass>
void FAGX_ConstraintUtilities::AddScrewControllerPropertyCallbacks(
	FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
	TFunction<FAGX_ConstraintScrewController*(UConstraintClass*)> GetController,
	const FName& Member)
{
	AddControllerPropertyCallbacks(PropertyDispatcher, GetController, Member);

	PropertyDispatcher.Add(
		Member, GET_MEMBER_NAME_CHECKED(FAGX_ConstraintScrewController, Lead),
		[GetController](UConstraintClass* EditedObject)
		{ GetController(EditedObject)->SetLead(GetController(EditedObject)->Lead); });
}

template AGXUNREAL_API void
FAGX_ConstraintUtilities::AddScrewControllerPropertyCallbacks<UAGX_Constraint2DofComponent>(
	FAGX_PropertyChangedDispatcher<UAGX_Constraint2DofComponent>& PropertyDispatcher,
	TFunction<FAGX_ConstraintScrewController*(UAGX_Constraint2DofComponent*)> GetController,
	const FName& Member);

#endif

namespace
{
	FVector GetGlobalAttachmentFramePos(UAGX_RigidBodyComponent* RigidBody, const FVector LocalPos)
	{
		if (RigidBody == nullptr)
		{
			// When RigidBody is nullptr the LocalPos is relative to the world.
			return LocalPos;
		}

		const FTransform BodyWorldTransform =
			FAGX_ObjectUtilities::GetAnyComponentWorldTransform(*RigidBody);

		return BodyWorldTransform.TransformPositionNoScale(LocalPos);
	}

	FQuat GetGlobalAttachmentFrameRot(UAGX_RigidBodyComponent* RigidBody, const FQuat LocalRot)
	{
		if (RigidBody == nullptr)
		{
			// When RigidBody is nullptr the LocalRot is relative to the world.
			return LocalRot;
		}

		const FTransform BodyWorldTransform =
			FAGX_ObjectUtilities::GetAnyComponentWorldTransform(*RigidBody);

		return BodyWorldTransform.TransformRotation(LocalRot);
	}
}

FTransform FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
	const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component,
	UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2,
	bool ForceOverwriteInstances)
{
	// Constraints are setup to use FrameDefiningSource == Constraint by default, meaning the
	// constraint itself is used to define the attachment frames. This means that we need to update
	// the transform of the constraint to be the same as the attachment frames (global) transform as
	// given by the barrier. One thing to note is that this is straight forward when the attachment
	// frames have the same global transform as each other. In the case that the constraint is
	// violated or rotated/translated along its degree(s) of freedom, there is no common transform
	// and therefore the constraint is always placed at the second attachment frame's
	// global transform. This means that the LocalFrameLocation/Rotation of BodyAttachment2 will be
	// zero by definition and that LocalFrameLocation/Rotation of BodyAttachment1 will reflect the
	// constraint violation and/or the translation/rotation along the degree(s) of freedom.
	// The reason that the constraint is placed at the second attachments frame instead of the first
	// is that if one were to describe a parent/child relationship between the two, the first
	// would be child and the second parent. This becomes apparent when considering creating an agx
	// constraint with new Constraint(body, Frame::Identity, nullptr, nullptr) where the second body
	// (nullptr) implicitly means the world. Also, the sign of the rotation/translation of the
	// secondary constraints support this ordering.

	AGX_COPY_PROPERTY_FROM(
		BodyAttachment1.FrameDefiningSource, EAGX_FrameDefiningSource::Constraint, Component,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		BodyAttachment2.FrameDefiningSource, EAGX_FrameDefiningSource::Constraint, Component,
		ForceOverwriteInstances)

	// Having a nullptr RigidBody1 is technically valid from AGX Dynamics perspective, but in >99%
	// of cases it is unexpected. Therefore, we opt to give the user a warning, and we still set the
	// FrameDefiningSource to be EAGX_FrameDefiningSource::Constraint before returning.
	if (RigidBody1 == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not setup Constraint frames since RigidBody1 was nullptr."));
		return FTransform::Identity;
	}

	const FVector Attach1GlobalPos =
		GetGlobalAttachmentFramePos(RigidBody1, Barrier.GetLocalLocation(0));
	const FQuat Attach1GlobalRot =
		GetGlobalAttachmentFrameRot(RigidBody1, Barrier.GetLocalRotation(0));
	const FVector Attach2GlobalPos =
		GetGlobalAttachmentFramePos(RigidBody2, Barrier.GetLocalLocation(1));
	const FQuat Attach2GlobalRot =
		GetGlobalAttachmentFrameRot(RigidBody2, Barrier.GetLocalRotation(1));

	// The Constraint's new World transform is the attachment frame 2.
	const FTransform NewWorldTransform(Attach2GlobalRot, Attach2GlobalPos);
	if (!FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		Component.SetWorldTransform(NewWorldTransform);
	}

	// The LocalFrameLocation and Rotation of BodyAttachment2 is always zero since the Constraint is
	// placed at the attachment frame 2.
	AGX_COPY_PROPERTY_FROM(
		BodyAttachment2.LocalFrameLocation, FVector::ZeroVector, Component, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		BodyAttachment2.LocalFrameRotation, FRotator(FQuat::Identity), Component,
		ForceOverwriteInstances)

	// The LocalFrameLocation and Rotation of BodyAttachment1 is the (global) attachment frame 1
	// expressed in the constraints (new) global frame.
	const FVector BodyAttachment1LocalLocation =
		NewWorldTransform.InverseTransformPositionNoScale(Attach1GlobalPos);
	const FRotator BodyAttachment1LocalRotation =
		FRotator(NewWorldTransform.InverseTransformRotation(Attach1GlobalRot));
	AGX_COPY_PROPERTY_FROM(
		BodyAttachment1.LocalFrameLocation, BodyAttachment1LocalLocation, Component,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		BodyAttachment1.LocalFrameRotation, BodyAttachment1LocalRotation, Component,
		ForceOverwriteInstances)

	return NewWorldTransform;
}

namespace FAGX_ConstraintUtilities_helpers
{
	/**
	 * Ensure that the attachment pair describe a valid constraint configuration. This means that
	 * the first body exists and has a valid native body, and that if the second body exists then
	 * it also has a native body.
	 *
	 * The native bodies are created if necessary.
	 *
	 * @param Attachment1 The attachment for the first body.
	 * @param Attachment2 The attachment for the second body or the world.
	 * @param ConstraintName Used only for error messages.
	 * @return True if the required native bodies are now available, false otherwise.
	 */
	bool EnsureValidConstraintAttachmentPair(
		FAGX_ConstraintBodyAttachment& Attachment1, FAGX_ConstraintBodyAttachment& Attachment2,
		const FName& ConstraintName)
	{
		FRigidBodyBarrier* Body1 = Attachment1.GetOrCreateRigidBodyBarrier();
		if (Body1 == nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(
					TEXT("Constraint %s: Could not get Rigid Body from Body Attachment 1. "
						 "Constraint cannot be created."),
					*ConstraintName.ToString()),
				SNotificationItem::CS_Fail);
			return false;
		}

		FRigidBodyBarrier* Body2 = Attachment2.GetOrCreateRigidBodyBarrier();
		if (Body2 == nullptr && Attachment2.GetRigidBody() != nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(
					TEXT("Constraint %s: A second body has been configured but it could not be "
						 "fetched. Constraint cannot be created."),
					*ConstraintName.ToString()),
				SNotificationItem::CS_Fail);
			return false;
		}

		return true;
	}

	FTransform GetFrameTransform(
		FAGX_ConstraintBodyAttachment& Attachment, const FName& ConstraintName,
		const FString& ActorLabel)
	{
		if (Attachment.FrameDefiningSource == EAGX_FrameDefiningSource::Other &&
			Attachment.FrameDefiningComponent.GetSceneComponent() == nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(
					TEXT("Constraint '%s' in Actor '%s' has Frame Defining Source set to Other but "
						 "Frame Defining Component is not set to a valid Component. Constraint "
						 "frames may be created incorrectly."),
					*ConstraintName.ToString(), *ActorLabel),
				SNotificationItem::CS_Fail);
		}

		if (Attachment.GetRigidBody() != nullptr)
		{
			const FVector Location = Attachment.GetLocalFrameLocationFromBody();
			const FQuat Rotation = Attachment.GetLocalFrameRotationFromBody();
			return FTransform(Rotation, Location);
		}
		else
		{
			const FVector Location = Attachment.GetGlobalFrameLocation();
			const FQuat Rotation = Attachment.GetGlobalFrameRotation();
			return FTransform(Rotation, Location);
		}
	}
}

void FAGX_ConstraintUtilities::CreateNative(
	FConstraintBarrier* Barrier, FAGX_ConstraintBodyAttachment& Attachment1,
	FAGX_ConstraintBodyAttachment& Attachment2, const FName& ConstraintName,
	const FString& ActorLabel)
{
	using namespace FAGX_ConstraintUtilities_helpers;

	if (Barrier == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Create Native failed for Constraint '%s', Barrier was nullptr"),
				*ConstraintName.ToString()),
			SNotificationItem::CS_Fail);
		return;
	}

	if (!EnsureValidConstraintAttachmentPair(Attachment1, Attachment2, ConstraintName))
	{
		// Error message is already printed.
		return;
	}

	FRigidBodyBarrier* Body1 = Attachment1.GetRigidBodyBarrier();
	FRigidBodyBarrier* Body2 = Attachment2.GetRigidBodyBarrier();

	FTransform Transform1 = GetFrameTransform(Attachment1, ConstraintName, ActorLabel);
	FTransform Transform2 = GetFrameTransform(Attachment2, ConstraintName, ActorLabel);
	Barrier->AllocateNative(
		*Body1, Transform1.GetLocation(), Transform1.GetRotation(), Body2, Transform2.GetLocation(),
		Transform2.GetRotation());
}
