#include "Constraints/AGX_ConstraintBodyAttachment.h"

#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "AGX_LogCategory.h"

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
		return FrameDefiningActor->GetActorTransform().TransformPositionNoScale(LocalFrameLocation);
	}
	else if (RigidBodyActor)
	{
		return RigidBodyActor->GetActorTransform().TransformPositionNoScale(LocalFrameLocation);
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

FMatrix FAGX_ConstraintBodyAttachment::GetGlobalFrameMatrix() const
{
	FQuat Rotation = GetGlobalFrameRotation();
	FVector Location = GetGlobalFrameLocation();
	return FMatrix(Rotation.GetAxisX(), Rotation.GetAxisY(), Rotation.GetAxisZ(), Location);
}

FRigidBodyBarrier* FAGX_ConstraintBodyAttachment::GetRigidBodyBarrier(bool CreateIfNeeded)
{
	if (!RigidBodyActor)
		return nullptr;

	UAGX_RigidBodyComponent* RigidBodyComponent =
		UAGX_RigidBodyComponent::GetFirstFromActor(RigidBodyActor);

	if (!RigidBodyComponent)
	{
		return nullptr;
	}

	if (CreateIfNeeded)
	{
		return RigidBodyComponent->GetOrCreateNative();
	}
	else
	{
		return RigidBodyComponent->GetNative();
	}
}

#if WITH_EDITOR

void FAGX_ConstraintBodyAttachment::OnFrameDefiningActorChanged(UAGX_ConstraintComponent* Parent)
{
	AAGX_ConstraintFrameActor* RecentConstraintFrame =
		Cast<AAGX_ConstraintFrameActor>(RecentFrameDefiningActor);
	AAGX_ConstraintFrameActor* ConstraintFrame =
		Cast<AAGX_ConstraintFrameActor>(FrameDefiningActor);

	RecentFrameDefiningActor = FrameDefiningActor;

	if (RecentConstraintFrame)
	{
		RecentConstraintFrame->RemoveConstraintUsage(Parent);
	}

	if (ConstraintFrame)
	{
		ConstraintFrame->AddConstraintUsage(Parent);
	}

	UE_LOG(
		LogAGX, Log,
		TEXT("OnFrameDefiningActorChanged: FrameDefiningActor = %s, ConstraintFrame = %s"),
		*GetNameSafe(FrameDefiningActor), *GetNameSafe(ConstraintFrame));
}

void FAGX_ConstraintBodyAttachment::OnDestroy(UAGX_ConstraintComponent* Parent)
{
	AAGX_ConstraintFrameActor* ConstraintFrame =
		Cast<AAGX_ConstraintFrameActor>(FrameDefiningActor);

	if (ConstraintFrame)
	{
		ConstraintFrame->RemoveConstraintUsage(Parent);
	}
}

#endif
