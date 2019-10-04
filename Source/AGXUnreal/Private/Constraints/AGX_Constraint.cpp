// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/AGX_Constraint.h"

#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"

#include "Constraints/ConstraintBarrier.h"


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

#endif


AAGX_Constraint::AAGX_Constraint()
{
	Compliance = 1.0E-8;
	Damping = 2.0 / 60.0;
}

AAGX_Constraint::~AAGX_Constraint()
{

}


#if WITH_EDITOR

void AAGX_Constraint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() :
		NAME_None;

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() :
		NAME_None;

	if (MemberPropertyName == PropertyName) // Property of this class changed
	{
	}
	else // Property of an aggregate struct changed
	{
		FAGX_ConstraintBodyAttachment* ModifiedBodyAttachment = 
			(MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAGX_Constraint, BodyAttachment1)) ? &BodyAttachment1 :
			((MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAGX_Constraint, BodyAttachment2)) ? &BodyAttachment2 :
			nullptr);

		if (ModifiedBodyAttachment)
		{
			if (PropertyName == GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, FrameDefiningActor))
			{
				// TODO: Code below needs to be triggered also when modified through code!
				// Editor-only probably OK though, since it is just for Editor convenience.
				// See FCoreUObjectDelegates::OnObjectPropertyChanged.
				// Or/additional add Refresh button to AAGX_ConstraintFrameActor's Details Panel
				// that rebuilds the constraint usage list.
				ModifiedBodyAttachment->OnFrameDefiningActorChanged(this);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PostEditChangeProperty: PropertyName = %s, MemberPropertyName = %s"),
		*PropertyName.ToString(), *MemberPropertyName.ToString());
}

#endif


FConstraintBarrier* AAGX_Constraint::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}


FConstraintBarrier* AAGX_Constraint::GetNative()
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}


bool AAGX_Constraint::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}


void AAGX_Constraint::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative())
	{
		CreateNative();
	}
}


void AAGX_Constraint::CreateNative()
{
	// TODO: Verify that we are in-game!

	check(!HasNative());
	
	CreateNativeImpl();

	// TODO: Shouldn't it be OK to continue if failed to initialize native (e.g. by lacking user setup)?
	// At least output a user error instead of crashing the program, since it is a user mistake
	// and not a code mistake to for example forgetting to assign a rigid body to the constraint!
	check(HasNative());

	NativeBarrier->SetCompliance(Compliance);
	NativeBarrier->SetDamping(Damping);

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);

	Simulation->GetNative()->AddConstraint(NativeBarrier.Get());
}