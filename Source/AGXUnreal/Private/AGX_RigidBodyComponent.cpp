#include "AGX_RigidBodyComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_ShapeComponent.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "Misc/EngineVersionComparison.h"

// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// We step the AGX Dynamics simulation in PrePhysics and read the new state
	// in PostPhysics. This doesn't really have anything to do with the PhysX
	// stepping, which is what the Physics tick groups really refer to, but the
	// names are instructive so we'll use them.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	Mass = 1.0f;
	PrincipalInertiae = FVector(1.f, 1.f, 1.f);
	MotionControl = EAGX_MotionControl::MC_DYNAMICS;
	TransformTarget = EAGX_TransformTarget::TT_SELF;
}

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		InitializeNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
	return &NativeBarrier;
}

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

bool UAGX_RigidBodyComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

void UAGX_RigidBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
}

/// \todo Split the UAGX_RigidBodyComponent::TickComponent callback into two
///       parts. One in PrePhysics that reads the Unreal state to AGX Dynamics
///       and one in PostPhysics that read the AGX Dynamics state to Unreal.
///       Read about tick splitting under Advanced Ticking Functionality at
///       https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Actors/Ticking/index.html
///
///      Take care to synchronize this with the actual AGX Dynamics stepping
///      done by UAGX_Simulation, they must not happen concurrently.
void UAGX_RigidBodyComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MotionControl != MC_STATIC)
	{
		ReadTransformFromNative();
		Velocity = NativeBarrier.GetVelocity();
		AngularVelocity = NativeBarrier.GetAngularVelocity();
	}
}

void UAGX_RigidBodyComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	NativeBarrier.ReleaseNative();
}

namespace
{
	TArray<UAGX_ShapeComponent*> GetShapes(const UAGX_RigidBodyComponent& Body)
	{
		/// \todo Do we want to search recursively? The user many build strange
		/// hierarchives, bodies beneath bodies and such, and care must be taken to
		/// get that right if we allow it. Only looking at immediate children
		/// simplifies things, but makes it impossible to attach shapes relative to
		/// each other. A middleground is to only search recursively within
		/// UAGX_ShapeComponents.
		return FAGX_ObjectUtilities::Filter<UAGX_ShapeComponent>(Body.GetAttachChildren());
	}
}

void UAGX_RigidBodyComponent::InitializeNative()
{
	NativeBarrier.AllocateNative();
	check(HasNative()); /// \todo Consider better error handling than 'check'.

	WritePropertiesToNative();
	WriteTransformToNative();

	for (UAGX_ShapeComponent* Shape : GetShapes(*this))
	{
		FShapeBarrier* NativeShape = Shape->GetOrCreateNative();
		/// \todo Should not crash on this. HeightField easy to get wrong.
		check(NativeShape && NativeShape->HasNative());
		NativeBarrier.AddShape(NativeShape);
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->AddRigidBody(this);

	if (bAutomaticMassProperties)
	{
		Mass = NativeBarrier.GetMassProperties().GetMass();
		PrincipalInertiae = NativeBarrier.GetMassProperties().GetPrincipalInertiae();
	}
}

void UAGX_RigidBodyComponent::WritePropertiesToNative()
{
	if (!HasNative())
	{
		return;
	}
	FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
	if (bAutomaticMassProperties)
	{
		MassProperties.SetAutoGenerate(bAutomaticMassProperties);
	}
	else
	{
		MassProperties.SetMass(Mass);
		MassProperties.SetPrincipalInertiae(PrincipalInertiae);
	}
	NativeBarrier.SetVelocity(Velocity);
	NativeBarrier.SetAngularVelocity(AngularVelocity);
	NativeBarrier.SetName(GetName());
	NativeBarrier.SetEnabled(bEnabled);
	InitializeMotionControl();
}

void UAGX_RigidBodyComponent::CopyFrom(const FRigidBodyBarrier& Barrier)
{
	const FMassPropertiesBarrier& MassProperties = Barrier.GetMassProperties();
	Mass = MassProperties.GetMass();
	PrincipalInertiae = MassProperties.GetPrincipalInertiae();
	bAutomaticMassProperties = MassProperties.GetAutoGenerate();
	Velocity = Barrier.GetVelocity();
	AngularVelocity = Barrier.GetAngularVelocity();
	MotionControl = Barrier.GetMotionControl();
	bEnabled = Barrier.GetEnabled();

// This breaks the move widget in Unreal Editor. Static bodies within Actors that have been
// imported from an AGX Dynamics archive does not move when the Actor is moved.
#if 0
	switch (MotionControl)
	{
		case MC_DYNAMICS:
		case MC_KINEMATICS:
			SetMobility(EComponentMobility::Movable);
			break;
		case MC_STATIC:
			SetMobility(EComponentMobility::Static);
			break;
	}
#endif

	/// \todo Should it always be SetWorld... here, or should we do SetRelative in some cases?
	SetWorldLocationAndRotation(Barrier.GetPosition(), Barrier.GetRotation());
}

void UAGX_RigidBodyComponent::InitializeMotionControl()
{
	NativeBarrier.SetMotionControl(MotionControl);

	if (MotionControl == MC_STATIC && Mobility != EComponentMobility::Static)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The Rigid Body Component \"%s\" has a RigidBody with Static AGX MotionControl "
				 "but Non-Static Unreal Mobility. Unreal Mobility will automatically be changed to "
				 "Static this game session, but should also be changed manually in the Editor to "
				 "ensure best performance!"),
			*GetName());

		SetMobility(EComponentMobility::Type::Static);
	}
	else if (MotionControl == MC_DYNAMICS && Mobility != EComponentMobility::Movable)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The Rigid Body Component \"%s\" has a RigidBody with Dynamic AGX MotionControl "
				 "but Non-Movable Unreal Mobility. Unreal Mobility will automatically be changed "
				 "to Movable this game session, but should also be changed manually in the Editor "
				 "to avoid future problems!"),
			*GetName());

		SetMobility(EComponentMobility::Type::Movable);
	}
}

void UAGX_RigidBodyComponent::ReadTransformFromNative()
{
	check(HasNative());
	const FVector NewLocation = NativeBarrier.GetPosition();
	const FQuat NewRotation = NativeBarrier.GetRotation();

	auto TransformSelf = [this, &NewLocation, &NewRotation]() {
		const FVector OldLocation = GetComponentLocation();
		const FVector LocationDelta = NewLocation - OldLocation;
		MoveComponent(LocationDelta, NewRotation, false);
	};

	auto TransformAncestor = [this, &NewLocation, &NewRotation](USceneComponent& Ancestor) {
		// Where Ancestor is relative to RigidBodyComponent, i.e., how the AGX Dynamics
		// transformation should be changed in order to be applicable to Ancestor.
		const FTransform AncestorRelativeToBody =
			Ancestor.GetComponentTransform().GetRelativeTransform(GetComponentTransform());

		// The transform we got from AGX Dynamics. We should manipulate Ancestor's transformation
		// so that this RigidBodyComponent end up at this position. All other children of
		// Ancestor should follow.
		const FTransform TargetBodyLocation = FTransform(NewRotation, NewLocation);

		// Compute the transform that moves Ancestor so that the body end up where we want it. Do
		// not change the scale of the ancestor, we don't want do deform meshes and other stuff in
		// there.
		FTransform NewTransform;
		FTransform::Multiply(&NewTransform, &AncestorRelativeToBody, &TargetBodyLocation);
		NewTransform.SetScale3D(Ancestor.GetComponentScale());

		Ancestor.SetWorldTransform(NewTransform);
	};

	auto TryTransformAncestor = [this, &NewLocation, &NewRotation,
								 &TransformAncestor](USceneComponent* Ancestor) {
		if (Ancestor == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot update transformation of ancestor of RigidBody '%s' because it "
					 "doesn't have an ancestor."),
				*GetName());
			return;
		}
		TransformAncestor(*Ancestor);
	};

	switch (TransformTarget)
	{
		case TT_SELF:
			TransformSelf();
			break;
		case TT_PARENT:
			TryTransformAncestor(GetAttachParent());
			break;
		case TT_ROOT:
			TryTransformAncestor(GetAttachmentRoot());
			break;
	}
}

void UAGX_RigidBodyComponent::WriteTransformToNative()
{
	check(HasNative());
	NativeBarrier.SetPosition(GetComponentLocation());
	NativeBarrier.SetRotation(GetComponentQuat());
}

#if WITH_EDITOR
bool UAGX_RigidBodyComponent::CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	const UProperty* InProperty
#else
	const FProperty* InProperty
#endif
) const
{
// This code was used when we had a bool property for the transform target and it used to enable
// or disable the checkbox in the Details Panel. Now that we have a drop-down list instead doing
// something like this is more complicated. Leaving the code here both as a reminder and for future
// inspiration.
//
// In essence, we want to disable the TT_ROOT option when TransformRootComponentAllowed returns
// false, and disable the TT_PARENT option when TransformParentComponentAllowed (not written yet)
// returns false.
#if 0
	// bTransformRootComponent is only allowed when this is the only RigidBodyComponent owned by the
	// parent actor.
	if (InProperty->GetFName() ==
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, bTransformRootComponent))
	{
		return TransformRootComponentAllowed();
	}
#endif
	return Super::CanEditChange(InProperty);
}

bool UAGX_RigidBodyComponent::TransformRootComponentAllowed() const
{
	if (GetOwner() == nullptr)
	{
		// Components don't have an owner while being built in a Blueprint. Not sure how to handle
		// this. Leaving it to the user for now, i.e., the user is responsible for not enabling
		// TransformRootComponent when there are multiple RigidBodyComponents in a Blueprint.
		return true;
	}
	return FAGX_ObjectUtilities::GetNumComponentsInActor<UAGX_RigidBodyComponent>(*GetOwner()) == 1;
}
#endif

/// \note Can use TInlineComponentArray<UAGX_RigidBodyComponent*> here, for performance.
TArray<UAGX_RigidBodyComponent*> UAGX_RigidBodyComponent::GetFromActor(const AActor* Actor)
{
	TArray<UAGX_RigidBodyComponent*> Bodies;
	if (Actor == nullptr)
	{
		return Bodies;
	}

	Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, false);
	return Bodies;
}

UAGX_RigidBodyComponent* UAGX_RigidBodyComponent::GetFirstFromActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UAGX_RigidBodyComponent>();
}

#if WITH_EDITOR
void UAGX_RigidBodyComponent::OnComponentView()
{
	/// @todo Here we should detect if TransformTarget has an illegal value for the current
	/// Component configuration in the Actor and if so set it back to TT_SELF.
	/// Or perhaps notify the user in some other way. Whatever gives the best UX.
	DisableTransformRootCompIfMultiple();
}

void UAGX_RigidBodyComponent::DisableTransformRootCompIfMultiple()
{
// This code was used when we had a bool property for the transform target and it used to forcibly
// disable root targeting when discovered that it was no longer legal due to multple bodies with
// the same Actor. This is a bit more complicated now that we have an enum instead of a bool, but
// the process should be similar.
//
// I'm leaving this code here both as a reminder and for future inspiration.
#if 0
	if (GetOwner() == nullptr)
	{
		// Components don't have an owner while being built in a Blueprint. This may actually be
		// a problem. Makes automatic bTransformRootComponent disabling impossible. Is there
		// another way to get a list of all sibling RigidBodyComponents?
		return;
	}

	/*
	 * TransformRootComponent is not allowed when the owning Actor has multiple RigidBodyComponents
	 * because the rigid bodies would then be fighting each others' transform synchronization from
	 * AGX Dynamics. This is true even if a single RigidBodyComponent has bTransformRootComponet set
	 * to true because that rigid body would wreck all the other bodies.
	 */
	TArray<UAGX_RigidBodyComponent*> Components;
	GetOwner()->GetComponents<UAGX_RigidBodyComponent>(Components, false);
	if (Components.Num() > 1)
	{
		// Disable the bTransformRootComponent flag for all UAGX_RigidBodyComponent in the owning
		// actor.
		for (auto C : Components)
		{
			if (C->bTransformRootComponent)
			{
				C->bTransformRootComponent = false;
			}
		}
	}
#endif
}
#endif

void UAGX_RigidBodyComponent::SetPosition(const FVector& Position)
{
	if (HasNative())
	{
		NativeBarrier.SetPosition(Position);
	}

	SetWorldLocation(Position);
}

FVector UAGX_RigidBodyComponent::GetPosition() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPosition();
	}

	return GetComponentLocation();
}

void UAGX_RigidBodyComponent::SetRotation(const FQuat& Rotation)
{
	if (HasNative())
	{
		NativeBarrier.SetRotation(Rotation);
	}

	SetWorldRotation(Rotation);
}

FQuat UAGX_RigidBodyComponent::GetRotation() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRotation();
	}

	return GetComponentQuat();
}

void UAGX_RigidBodyComponent::SetEnabled(bool InEnabled)
{
	if (HasNative())
	{
		NativeBarrier.SetEnabled(InEnabled);
	}

	bEnabled = InEnabled;
}

bool UAGX_RigidBodyComponent::GetEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnabled();
	}

	return bEnabled;
}

void UAGX_RigidBodyComponent::SetAutomaticMassProperties(bool InEnabled)
{
	if (HasNative())
	{
		FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
		MassProperties.SetAutoGenerate(InEnabled);
	}

	bAutomaticMassProperties = InEnabled;
}

bool UAGX_RigidBodyComponent::GetAutomaticMassProperties() const
{
	if (HasNative())
	{
		const FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
		return MassProperties.GetAutoGenerate();
	}

	return bAutomaticMassProperties;
}

void UAGX_RigidBodyComponent::SetMass(float InMass)
{
	if (HasNative())
	{
		FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
		MassProperties.SetMass(InMass);
	}

	Mass = InMass;
}

float UAGX_RigidBodyComponent::GetMass() const
{
	if (HasNative())
	{
		const FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
		return MassProperties.GetMass();
	}

	return Mass;
}

void UAGX_RigidBodyComponent::SetPrincipalInertiae(const FVector& InPrincipalInertiae)
{
	if (HasNative())
	{
		FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
		MassProperties.SetPrincipalInertiae(InPrincipalInertiae);
	}

	PrincipalInertiae = InPrincipalInertiae;
}

FVector UAGX_RigidBodyComponent::GetPrincipalInertiae() const
{
	if (HasNative())
	{
		const FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
		return MassProperties.GetPrincipalInertiae();
	}

	return PrincipalInertiae;
}

void UAGX_RigidBodyComponent::SetVelocity(const FVector& InVelocity)
{
	if (HasNative())
	{
		NativeBarrier.SetVelocity(InVelocity);
	}

	Velocity = InVelocity;
}

FVector UAGX_RigidBodyComponent::GetVelocity() const
{
	if (HasNative())
	{
		return NativeBarrier.GetVelocity();
	}

	return Velocity;
}

void UAGX_RigidBodyComponent::SetAngularVelocity(const FVector& InAngularVelocity)
{
	if (HasNative())
	{
		NativeBarrier.SetAngularVelocity(InAngularVelocity);
	}

	AngularVelocity = InAngularVelocity;
}

FVector UAGX_RigidBodyComponent::GetAngularVelocity() const
{
	if (HasNative())
	{
		return NativeBarrier.GetAngularVelocity();
	}

	return AngularVelocity;
}

void UAGX_RigidBodyComponent::SetMotionControl(TEnumAsByte<enum EAGX_MotionControl> InMotionControl)
{
	if (HasNative())
	{
		NativeBarrier.SetMotionControl(InMotionControl);
	}

	MotionControl = InMotionControl;
}

TEnumAsByte<enum EAGX_MotionControl> UAGX_RigidBodyComponent::GetMotionControl() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMotionControl();
	}

	return MotionControl;
}
