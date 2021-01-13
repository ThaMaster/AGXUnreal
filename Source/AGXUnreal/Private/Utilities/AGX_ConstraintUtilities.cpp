#include "Utilities/AGX_ConstraintUtilities.h"

// AGX Dynamics for Unreal includes.
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
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_Constraint1DofComponent& Component, const FConstraint1DOFBarrier& Barrier)
{
	StoreElectricMotorController(Barrier, Component.ElectricMotorController);
	StoreFrictionController(Barrier, Component.FrictionController);
	StoreLockController(Barrier, Component.LockController);
	StoreRangeController(Barrier, Component.RangeController);
	StoreTargetSpeedController(Barrier, Component.TargetSpeedController);
}

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_Constraint2DofComponent& Component, const FConstraint2DOFBarrier& Barrier)
{
	const EAGX_Constraint2DOFFreeDOF First = EAGX_Constraint2DOFFreeDOF::FIRST;
	const EAGX_Constraint2DOFFreeDOF Second = EAGX_Constraint2DOFFreeDOF::SECOND;

	StoreElectricMotorController(Barrier, Component.ElectricMotorController1, First);
	StoreElectricMotorController(Barrier, Component.ElectricMotorController2, Second);

	StoreFrictionController(Barrier, Component.FrictionController1, First);
	StoreFrictionController(Barrier, Component.FrictionController2, Second);

	StoreLockController(Barrier, Component.LockController1, First);
	StoreLockController(Barrier, Component.LockController2, Second);

	StoreRangeController(Barrier, Component.RangeController1, First);
	StoreRangeController(Barrier, Component.RangeController2, Second);

	StoreTargetSpeedController(Barrier, Component.TargetSpeedController1, First);
	StoreTargetSpeedController(Barrier, Component.TargetSpeedController2, Second);
}

void FAGX_ConstraintUtilities::CopyControllersFrom(
	UAGX_ConstraintComponent& Component, const FConstraintBarrier& Barrier)
{
	// This exists only to provide a complete overload resolution set for the constraint types. Only
	// 1Dof and 2Dof constraints, the other overloads, have any controllers to store. Make sure you
	// end up in the correct overload of CopyControllersFrom for the type of constraint you actually
	// have.
	/// \todo Consider making this a virtual member function instead, to avoid ending up in this
	/// empty version unintentionally.
}

void FAGX_ConstraintUtilities::StoreElectricMotorController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller)
{
	Controller.CopyFrom(*Barrier.GetElectricMotorController());
}

void FAGX_ConstraintUtilities::StoreElectricMotorController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetElectricMotorController(Dof));
}

void FAGX_ConstraintUtilities::StoreFrictionController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller)
{
	Controller.CopyFrom(*Barrier.GetFrictionController());
}

void FAGX_ConstraintUtilities::StoreFrictionController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetFrictionController(Dof));
}

void FAGX_ConstraintUtilities::StoreLockController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller)
{
	Controller.CopyFrom(*Barrier.GetLockController());
}

void FAGX_ConstraintUtilities::StoreLockController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetLockController(Dof));
}

void FAGX_ConstraintUtilities::StoreRangeController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller)
{
	Controller.CopyFrom(*Barrier.GetRangeController());
}

void FAGX_ConstraintUtilities::StoreRangeController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetRangeController(Dof));
}

void FAGX_ConstraintUtilities::StoreTargetSpeedController(
	const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller)
{
	Controller.CopyFrom(*Barrier.GetTargetSpeedController());
}

void FAGX_ConstraintUtilities::StoreTargetSpeedController(
	const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
	EAGX_Constraint2DOFFreeDOF Dof)
{
	Controller.CopyFrom(*Barrier.GetTargetSpeedController(Dof));
}

namespace
{
	FVector GetGlobalAttachmentFramePos(UAGX_RigidBodyComponent* RigidBody, const FVector LocalPos)
	{
		if (RigidBody)
		{
			return RigidBody->GetComponentTransform().TransformPositionNoScale(LocalPos);
		}

		// When RigidBody is nullptr the LocalPos is relative to the world.
		return LocalPos;
	}

	FQuat GetGlobalAttachmentFrameRot(UAGX_RigidBodyComponent* RigidBody, const FQuat LocalRot)
	{
		if (RigidBody)
		{
			return RigidBody->GetComponentTransform().TransformRotation(LocalRot);
		}

		// When RigidBody is nullptr the LocalRot is relative to the world.
		return LocalRot;
	}
}

void FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
	const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component,
	UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2)
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

	if (!RigidBody1)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not setup Constraint frames since RigidBody1 was nullptr."));
		return;
	}

	Component.BodyAttachment1.FrameDefiningSource = EAGX_FrameDefiningSource::Constraint;
	Component.BodyAttachment2.FrameDefiningSource = EAGX_FrameDefiningSource::Constraint;

	const FVector Attach1GlobalPos =
		GetGlobalAttachmentFramePos(RigidBody1, Barrier.GetLocalLocation(0));
	const FQuat Attach1GlobalRot =
		GetGlobalAttachmentFrameRot(RigidBody1, Barrier.GetLocalRotation(0));
	const FVector Attach2GlobalPos =
		GetGlobalAttachmentFramePos(RigidBody2, Barrier.GetLocalLocation(1));
	const FQuat Attach2GlobalRot =
		GetGlobalAttachmentFrameRot(RigidBody2, Barrier.GetLocalRotation(1));

	// Set the Constraint's transform same as attachment frame 2.
	Component.SetWorldLocationAndRotation(Attach2GlobalPos, Attach2GlobalRot);

	// The LocalFrameLocation and Rotation of BodyAttachment2 is always zero since the Constraint is
	// placed at the attachment frame 2.
	Component.BodyAttachment2.LocalFrameLocation = FVector::ZeroVector;
	Component.BodyAttachment2.LocalFrameRotation = FRotator(FQuat::Identity);

	// The LocalFrameLocation and Rotation of BodyAttachment1 is the (global) attachment frame 1
	// expressed in the constraints (new) global frame.
	Component.BodyAttachment1.LocalFrameLocation =
		Component.GetComponentTransform().InverseTransformPositionNoScale(Attach1GlobalPos);
	Component.BodyAttachment1.LocalFrameRotation =
		FRotator(Component.GetComponentTransform().InverseTransformRotation(Attach1GlobalRot));
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
			UE_LOG(
				LogAGX, Error,
				TEXT("Constraint %s: Could not get Rigid Body from Body Attachment 1. "
					 "Constraint cannot be created."),
				*ConstraintName.ToString());
			return false;
		}
		// Rename to GetOrCreate
		FRigidBodyBarrier* Body2 = Attachment2.GetOrCreateRigidBodyBarrier();
		if (Body2 == nullptr && Attachment2.GetRigidBody() != nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Constraint %s: A second body has been configured but it could not be "
					 "fetched. Constraint cannot be created."),
				*ConstraintName.ToString());
			return false;
		}

		return true;
	}

	FTransform GetFrameTransform(FAGX_ConstraintBodyAttachment& Attachment)
	{
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
	FAGX_ConstraintBodyAttachment& Attachment2, const FName& ConstraintName)
{
	using namespace FAGX_ConstraintUtilities_helpers;

	if (Barrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Create Native failed for Constraint %s, Barrier was nullptr"),
			*ConstraintName.ToString());
		return;
	}

	if (!EnsureValidConstraintAttachmentPair(Attachment1, Attachment2, ConstraintName))
	{
		// Error message is already printed.
		return;
	}

	FRigidBodyBarrier* Body1 = Attachment1.GetRigidBodyBarrier();
	FRigidBodyBarrier* Body2 = Attachment2.GetRigidBodyBarrier();

	FTransform Transform1 = GetFrameTransform(Attachment1);
	FTransform Transform2 = GetFrameTransform(Attachment2);
	Barrier->AllocateNative(
		*Body1, Transform1.GetLocation(), Transform1.GetRotation(), Body2, Transform2.GetLocation(),
		Transform2.GetRotation());
}
