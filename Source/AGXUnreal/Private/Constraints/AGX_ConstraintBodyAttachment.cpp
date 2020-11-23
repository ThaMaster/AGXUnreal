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

FAGX_ConstraintBodyAttachment::FAGX_ConstraintBodyAttachment()
{
	bCanEditFrameDefiningComponent = FrameDefiningSource == EAGX_FrameDefiningSource::OTHER;
}

FAGX_ConstraintBodyAttachment::FAGX_ConstraintBodyAttachment(USceneComponent* InOwner)
	: Owner {InOwner}
{
	bCanEditFrameDefiningComponent = FrameDefiningSource == EAGX_FrameDefiningSource::OTHER;
}

UAGX_RigidBodyComponent* FAGX_ConstraintBodyAttachment::GetRigidBody() const
{
	return RigidBody.GetRigidBody();
}

FVector FAGX_ConstraintBodyAttachment::GetLocalFrameLocationFromBody() const
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetLocalFrameLocationFromBody() called on AGX_ConstraintBodyAttachment whose "
				 "RigidBody is not set which may lead to unwanted behaviour. The FrameLocation "
				 "retuned will be given in world coordinate system."));
		return LocalFrameLocation;
	}

	// If the FrameDefiningSource is RIGIDBODY, the LocalFrameLocation is already given in RigidBody's
	// frame by definition and we can simply return the value directly.
	if (FrameDefiningSource == EAGX_FrameDefiningSource::RIGIDBODY)
	{
		return LocalFrameLocation;
	}

	return Body->GetComponentTransform().InverseTransformPositionNoScale(GetGlobalFrameLocation());
}

FQuat FAGX_ConstraintBodyAttachment::GetLocalFrameRotationFromBody() const
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetLocalFrameRotationFromBody() called on AGX_ConstraintBodyAttachment whose "
				 "RigidBody is not set which may lead to unwanted behaviour. The LocalFrameRotation "
				 "retuned will be given in world coordinate system."));
		return LocalFrameRotation.Quaternion();
	}

	// If the FrameDefiningSource is RIGIDBODY, the LocalFrameRotation is already given in RigidBody's
	// frame by definition and we can simply return the value directly.
	if (FrameDefiningSource == EAGX_FrameDefiningSource::RIGIDBODY)
	{
		return LocalFrameRotation.Quaternion();
	}

	return Body->GetComponentTransform().InverseTransformRotation(GetGlobalFrameRotation());
}

FVector FAGX_ConstraintBodyAttachment::GetGlobalFrameLocation() const
{
	if (USceneComponent* FrameDefiningComp = GetFinalFrameDefiningComponent())
	{
		return FrameDefiningComp->GetComponentTransform().TransformPositionNoScale(
			LocalFrameLocation);
	}

	return LocalFrameLocation;
}

FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation() const
{
	if (USceneComponent * FrameDefiningComp = GetFinalFrameDefiningComponent())
	{
		return FrameDefiningComp->GetComponentTransform().TransformRotation(
			LocalFrameRotation.Quaternion());
	}

	return LocalFrameRotation.Quaternion();
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

void FAGX_ConstraintBodyAttachment::OnFrameDefiningSourceChanged()
{
	bCanEditFrameDefiningComponent = FrameDefiningSource == EAGX_FrameDefiningSource::OTHER;
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

USceneComponent* FAGX_ConstraintBodyAttachment::GetFinalFrameDefiningComponent() const
{
	switch (FrameDefiningSource)
	{
		case CONSTRAINT:
			return Owner;
		case RIGIDBODY:
			return GetRigidBody();
		case OTHER:
			return FrameDefiningComponent.GetSceneComponent();
	}

	return nullptr;
}
#endif
