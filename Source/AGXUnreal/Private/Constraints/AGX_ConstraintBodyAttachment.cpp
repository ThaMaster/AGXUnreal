#include "Constraints/AGX_ConstraintBodyAttachment.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

UAGX_RigidBodyComponent* FAGX_ConstraintBodyAttachment::GetRigidBodyComponent() const
{
	USceneComponent* Component = RigidBodyComponent.GetComponent(nullptr);
	return Cast<UAGX_RigidBodyComponent>(Component);
}

FVector FAGX_ConstraintBodyAttachment::GetLocalFrameLocation() const
{
#if AGX_UNREAL_RIGID_BODY_COMPONENT
	UAGX_RigidBodyComponent* Body = GetRigidBodyComponent();
	if (Body == nullptr)
	{
		/// \todo Someone is using a FAGX_ConstraintBodyAttachment that isn't
		/// attached to any body. It's unclear to me what that really means, and if
		/// it should be legal or not. Logging for now, but remove the logging when
		/// we find a case where such attachments makes sense.
		UE_LOG(
			LogAGX, Warning,
			TEXT("Something is getting the local location of a ConstraintBodyAttachment without an "
				 "attached body. Unclear what the result should be."));
		return LocalFrameLocation;
	}

	/// \todo This does a pointless transform/inversetransform if there is no
	/// frame defining actor. Detect that case and just return LocalFrameLocation.

	return Body->GetComponentTransform().InverseTransformPositionNoScale(GetGlobalFrameLocation());
#else
	if (RigidBodyActor && FrameDefiningActor)
	{
		return RigidBodyActor->GetActorTransform().InverseTransformPositionNoScale(
			GetGlobalFrameLocation());
	}
	else
	{
		return LocalFrameLocation; // already defined relative to rigid body or world
	}
#endif
}

FQuat FAGX_ConstraintBodyAttachment::GetLocalFrameRotation() const
{
#if AGX_UNREAL_RIGID_BODY_COMPONENT
	UAGX_RigidBodyComponent* Body = GetRigidBodyComponent();
	if (Body == nullptr)
	{
		return LocalFrameRotation.Quaternion();
	}

	return Body->GetComponentTransform().InverseTransformRotation(GetGlobalFrameRotation());
#else
	if (RigidBodyActor && FrameDefiningActor)
	{
		return RigidBodyActor->GetActorTransform().InverseTransformRotation(
			GetGlobalFrameRotation());
	}
	else
	{
		return LocalFrameRotation.Quaternion(); // already defined relative to rigid body or world
	}
#endif
}

FVector FAGX_ConstraintBodyAttachment::GetGlobalFrameLocation() const
{
#if AGX_UNREAL_RIGID_BODY_COMPONENT
	if (FrameDefiningActor != nullptr)
	{
		/// \todo What is the effect of NoScale here? What happens if ancestor
		/// transformations contain scales? All all scales ignored?
		return FrameDefiningActor->GetActorTransform().TransformPositionNoScale(LocalFrameLocation);
	}
	else if (UAGX_RigidBodyComponent* Body = GetRigidBodyComponent())
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
#else
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
#endif
}

FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation() const
{
#if AGX_UNREAL_RIGID_BODY_COMPONENT
	if (FrameDefiningActor != nullptr)
	{
		return FrameDefiningActor->GetActorTransform().TransformRotation(
			LocalFrameRotation.Quaternion());
	}
	else if (UAGX_RigidBodyComponent* Body = GetRigidBodyComponent())
	{
		return Body->GetComponentTransform().TransformRotation(LocalFrameRotation.Quaternion());
	}
	else
	{
		// When there is nothing that the local rotation is relative to then we
		// assume it is a global rotation as well.
		return LocalFrameRotation.Quaternion();
	}
#else
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
#endif
}

FMatrix FAGX_ConstraintBodyAttachment::GetGlobalFrameMatrix() const
{
	FQuat Rotation = GetGlobalFrameRotation();
	FVector Location = GetGlobalFrameLocation();
	return FMatrix(Rotation.GetAxisX(), Rotation.GetAxisY(), Rotation.GetAxisZ(), Location);
}

FRigidBodyBarrier* FAGX_ConstraintBodyAttachment::GetRigidBodyBarrier(bool CreateIfNeeded)
{
#if AGX_UNREAL_RIGID_BODY_COMPONENT
	UAGX_RigidBodyComponent* Body = GetRigidBodyComponent();
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
#else
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
#endif
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
