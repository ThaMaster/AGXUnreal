#include "Constraints/AGX_ConstraintBodyAttachment.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

UAGX_RigidBodyComponent* FAGX_ConstraintBodyAttachment::GetRigidBody() const
{
	return RigidBody.GetRigidBody();
}

FVector FAGX_ConstraintBodyAttachment::GetLocalFrameLocation() const
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		/// \todo Someone is using a FAGX_ConstraintBodyAttachment that isn't
		/// attached to any body. It's unclear to me what that really means, and if
		/// it should be legal or not. Logging for now, but remove the logging when
		/// we find a case where such attachments makes sense.
		UE_LOG(
			LogAGX, Warning,
			TEXT("Something is getting the local location of a ConstraintBodyAttachment without an "
				 "attached body. May produce unwanted behavior."));
		return LocalFrameLocation;
	}

	/// \todo This does a pointless transform/inversetransform if there is no
	/// frame defining actor. Detect that case and just return LocalFrameLocation.

	return Body->GetComponentTransform().InverseTransformPositionNoScale(GetGlobalFrameLocation());
}

FQuat FAGX_ConstraintBodyAttachment::GetLocalFrameRotation() const
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		return LocalFrameRotation.Quaternion();
	}

	return Body->GetComponentTransform().InverseTransformRotation(GetGlobalFrameRotation());
}

FVector FAGX_ConstraintBodyAttachment::GetGlobalFrameLocation() const
{
	if (FrameDefiningActor != nullptr)
	{
		return FrameDefiningActor->GetActorTransform().TransformPositionNoScale(LocalFrameLocation);
	}
	else if (UAGX_RigidBodyComponent* Body = GetRigidBody())
	{
		return Body->GetComponentTransform().TransformPositionNoScale(LocalFrameLocation);
	}
	else
	{
		// When there is nothing that the local location is relative to then we
		// assume it is a global location as well.
		/// \todo When would that ever happen?
		return LocalFrameLocation;
	}
}

FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation() const
{
	if (FrameDefiningActor != nullptr)
	{
		return FrameDefiningActor->GetActorTransform().TransformRotation(
			LocalFrameRotation.Quaternion());
	}
	else if (UAGX_RigidBodyComponent* Body = GetRigidBody())
	{
		return Body->GetComponentTransform().TransformRotation(LocalFrameRotation.Quaternion());
	}
	else
	{
		// When there is nothing that the local rotation is relative to then we
		// assume it is a global rotation.
		return LocalFrameRotation.Quaternion();
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
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		return nullptr;
	}

	FRigidBodyBarrier* Barrier = Body->GetNative();
	if (Barrier == nullptr && CreateIfNeeded)
	{
		Barrier = Body->GetOrCreateNative();
	}
	return Barrier;
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
}

void FAGX_ConstraintBodyAttachment::OnRigidBodyReferenceChanged()
{
	UE_LOG(
		LogAGX, Error, TEXT("FAGX_ConstraintBodyAttachment::OnRigidBodyReferenceChanged called."));

	/// \todo Check if an illegal, i.e., not UAGX_RigidBodyComponent, component has been selected.
	/// Selecting nothing is not an error.
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
