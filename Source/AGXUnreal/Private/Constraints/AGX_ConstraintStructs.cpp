#include "AGX_ConstraintStructs.h"

#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"

#include "Constraints/ControllerConstraintBarriers.h"


FVector FAGX_ConstraintBodyAttachment::GetLocalFrameLocation() const
{
	if (RigidBodyActor && FrameDefiningActor)
	{
		return RigidBodyActor->GetActorTransform().InverseTransformPositionNoScale(
			GetGlobalFrameLocation());
	}
	else
	{
		return LocalFrameLocation; // already defined relative to rigid body or world
	}
}


FQuat FAGX_ConstraintBodyAttachment::GetLocalFrameRotation() const
{
	if (RigidBodyActor && FrameDefiningActor)
	{
		return RigidBodyActor->GetActorTransform().InverseTransformRotation(
			GetGlobalFrameRotation());
	}
	else
	{
		return LocalFrameRotation.Quaternion(); // already defined relative to rigid body or world
	}
}


FVector FAGX_ConstraintBodyAttachment::GetGlobalFrameLocation() const
{
	if (FrameDefiningActor)
	{
		return FrameDefiningActor->GetActorTransform().TransformPositionNoScale(
			LocalFrameLocation);
	}
	else if (RigidBodyActor)
	{
		return RigidBodyActor->GetActorTransform().TransformPositionNoScale(
			LocalFrameLocation);
	}
	else
	{
		return LocalFrameLocation; // already defined in world space
	}
}


FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation() const
{
	if (FrameDefiningActor)
	{
		return FrameDefiningActor->GetActorTransform().TransformRotation(
			LocalFrameRotation.Quaternion());
	}
	else if (RigidBodyActor)
	{
		return RigidBodyActor->GetActorTransform().TransformRotation(
			LocalFrameRotation.Quaternion());
	}
	else
	{
		return LocalFrameRotation.Quaternion(); // already defined in world space
	}
}


FRigidBodyBarrier* FAGX_ConstraintBodyAttachment::GetRigidBodyBarrier(bool CreateIfNeeded)
{
	if (!RigidBodyActor)
		return nullptr;

	UAGX_RigidBodyComponent* RigidBodyComponent =
		UAGX_RigidBodyComponent::GetFromActor(RigidBodyActor);

	if (!RigidBodyComponent)
		return nullptr;

	if (CreateIfNeeded)
		return RigidBodyComponent->GetOrCreateNative();
	else
		return RigidBodyComponent->GetNative();
}


#if WITH_EDITOR

void FAGX_ConstraintBodyAttachment::OnFrameDefiningActorChanged(AAGX_Constraint* Owner)
{
	AAGX_ConstraintFrameActor* RecentConstraintFrame = Cast<AAGX_ConstraintFrameActor>(RecentFrameDefiningActor);
	AAGX_ConstraintFrameActor* ConstraintFrame = Cast<AAGX_ConstraintFrameActor>(FrameDefiningActor);

	RecentFrameDefiningActor = FrameDefiningActor;

	if (RecentConstraintFrame)
		RecentConstraintFrame->RemoveConstraintUsage(Owner);

	if (ConstraintFrame)
		ConstraintFrame->AddConstraintUsage(Owner);


	UE_LOG(LogTemp, Log, TEXT("OnFrameDefiningActorChanged: FrameDefiningActor = %s, ConstraintFrame = %s"),
		*GetNameSafe(FrameDefiningActor),
		*GetNameSafe(ConstraintFrame));
}

void FAGX_ConstraintBodyAttachment::OnDestroy(AAGX_Constraint* Owner)
{
	AAGX_ConstraintFrameActor* ConstraintFrame = Cast<AAGX_ConstraintFrameActor>(FrameDefiningActor);

	if (ConstraintFrame)
		ConstraintFrame->RemoveConstraintUsage(Owner);
}

#endif

#define DEFAULT_SECONDARY_COMPLIANCE 1.0e-10
#define DEFAULT_SECONDARY_ELASTICITY (1.0 / DEFAULT_SECONDARY_COMPLIANCE)
#define DEFAULT_SECONDARY_DAMPING (2.0 / 60.0)
#define RANGE_LOWEST_FLOAT TNumericLimits<float>::Lowest()
#define RANGE_HIGHEST_FLOAT TNumericLimits<float>::Max()


FAGX_ConstraintElectricMotorController::FAGX_ConstraintElectricMotorController(bool bRotational_)
	:
	bEnable(false),
	Voltage(24.0),
	ArmatureResistance(1.0),
	TorqueConstant(1.0),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintElectricMotorController::ToBarrier(FElectricMotorControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->Voltage = Voltage;
	Barrier->ArmatureResistance = ArmatureResistance;
	Barrier->TorqueConstant = TorqueConstant;
}


FAGX_ConstraintFrictionController::FAGX_ConstraintFrictionController(bool bRotational_)
	:
	bEnable(false),
	FrictionCoefficient(0.416667),
	bEnableNonLinearDirectSolveUpdate(false),
	Elasticity(1.0e8),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintFrictionController::ToBarrier(FFrictionControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->FrictionCoefficient = FrictionCoefficient;
	Barrier->bEnableNonLinearDirectSolveUpdate = bEnableNonLinearDirectSolveUpdate;
}


FAGX_ConstraintLockController::FAGX_ConstraintLockController(bool bRotational_)
	:
	bEnable(false),
	Position(0.0),
	Elasticity(1.0e8),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintLockController::ToBarrier(FLockControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->Position = bRotational ? FMath::DegreesToRadians(Position) : Position;
}


FAGX_ConstraintRangeController::FAGX_ConstraintRangeController(bool bRotational_)
	:
	bEnable(false),
	Range({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	Elasticity(DEFAULT_SECONDARY_ELASTICITY),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ 0.0, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintRangeController::ToBarrier(FRangeControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->RangeMin = bRotational ? FMath::DegreesToRadians(Range.Min) : Range.Min;
	Barrier->RangeMax = bRotational ? FMath::DegreesToRadians(Range.Max) : Range.Max;
}


FAGX_ConstraintTargetSpeedController::FAGX_ConstraintTargetSpeedController(bool bRotational_)
	:
	bEnable(false),
	Speed(0.0),
	bLockedAtZeroSpeed(false),
	Elasticity(1.0e8),
	Damping(DEFAULT_SECONDARY_DAMPING),
	ForceRange({ RANGE_LOWEST_FLOAT, RANGE_HIGHEST_FLOAT }),
	bRotational(bRotational_)
{

}


void FAGX_ConstraintTargetSpeedController::ToBarrier(FTargetSpeedControllerBarrier* Barrier) const
{
	if (!Barrier)
		return;

	Barrier->bEnable = bEnable;
	Barrier->Elasticity = Elasticity;
	Barrier->Damping = Damping;
	Barrier->ForceRangeMin = ForceRange.Min;
	Barrier->ForceRangeMax = ForceRange.Max;

	Barrier->bRotational = bRotational;

	Barrier->Speed = bRotational ? FMath::DegreesToRadians(Speed) : Speed;
	Barrier->bLockedAtZeroSpeed = bLockedAtZeroSpeed;
}
