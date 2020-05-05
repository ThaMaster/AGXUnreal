#include "Constraints/AGX_ConstraintBodyAttachment.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_SceneComponentReference.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "UObject/UObjectGlobals.h"

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
	/// \todo Is it safe to replace this code with a call to GetGlobalFrameLocation(GetRigidBody())?
	/// The difference would be that GetRigidBody would be called in cases where it would not
	/// before.

	if (USceneComponent* Origin = FrameDefiningComponent.GetSceneComponent())
	{
		return Origin->GetComponentTransform().TransformPositionNoScale(LocalFrameLocation);
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

FVector FAGX_ConstraintBodyAttachment::GetGlobalFrameLocation(UAGX_RigidBodyComponent* Body) const
{
	if (USceneComponent* Origin = FrameDefiningComponent.GetSceneComponent())
	{
		return Origin->GetComponentTransform().TransformPositionNoScale(LocalFrameLocation);
	}
	else if (Body != nullptr)
	{
		return Body->GetComponentTransform().TransformPositionNoScale(LocalFrameLocation);
	}
	else
	{
		// When there is nothing that the local location is relative to then we assume it is a
		// global location as well.
		return LocalFrameLocation;
	}
}

FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation() const
{
	/// \todo Is it safe to replace this code with a call to GetGlobalFrameRotation(GetRigidBody())?
	/// The difference would be that GetRigidBody would be called in cases where it would not
	/// before.

	if (USceneComponent* Origin = FrameDefiningComponent.GetSceneComponent())
	{
		return Origin->GetComponentTransform().TransformRotation(LocalFrameRotation.Quaternion());
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

FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation(UAGX_RigidBodyComponent* Body) const
{
	if (USceneComponent* Origin = FrameDefiningComponent.GetSceneComponent())
	{
		return Origin->GetComponentTransform().TransformRotation(LocalFrameRotation.Quaternion());
	}
	else if (Body != nullptr)
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

FMatrix FAGX_ConstraintBodyAttachment::GetGlobalFrameMatrix(UAGX_RigidBodyComponent* Body) const
{
	FQuat Rotation = GetGlobalFrameRotation(Body);
	FVector Location = GetGlobalFrameLocation(Body);
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

void FAGX_ConstraintBodyAttachment::OnFrameDefiningComponentChanged(
	UAGX_ConstraintComponent* Parent)
{
	UAGX_ConstraintFrameComponent* Previous =
		Cast<UAGX_ConstraintFrameComponent>(PreviousFrameDefiningComponent);
	UAGX_ConstraintFrameComponent* Next =
		Cast<UAGX_ConstraintFrameComponent>(FrameDefiningComponent.GetSceneComponent());

	PreviousFrameDefiningComponent = FrameDefiningComponent.GetSceneComponent();

	if (Previous)
	{
		Previous->RemoveConstraintUsage(Parent);
	}
	if (Next)
	{
		Next->AddConstraintUsage(Parent);
	}
}

void FAGX_ConstraintBodyAttachment::OnDestroy(UAGX_ConstraintComponent* Parent)
{
	// This may be a problem. FrameDefiningComponent uses a TSoftObjectPtr to reference the
	// SceneComponent. This is required for it to be usabel in the Mode Panel and Blueprint editors.
	// It is not legal to dereference a TSoftObjectPtr during  GarbageCollection, and OnDestroy is
	// likely to be called during GarbageCollection. The assert message from Unreal Engine is
	//     Illegal call to StaticFindObject() while collecting garbage!
	if (IsGarbageCollecting())
	{
		return;
	}

	UAGX_ConstraintFrameComponent* ConstraintFrame =
		Cast<UAGX_ConstraintFrameComponent>(FrameDefiningComponent.GetSceneComponent());

	if (ConstraintFrame)
	{
		ConstraintFrame->RemoveConstraintUsage(Parent);
	}
}

#endif
