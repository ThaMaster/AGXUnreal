#include "AGX_RigidBodyComponent.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_ObjectUtilities.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_ShapeComponent.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	UE_LOG(LogAGX, Log, TEXT("RigidBody instance created."));
	PrimaryComponentTick.bCanEverTick = true;

	// We step the AGX Dynamics simulation in PrePhysics and read the new state
	// in PostPhysics. This doesn't really have anything to do with the PhysX
	// stepping, which is what the Physics tick groups really refer to, but the
	// names are instructive so we'll use them.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	Mass = 1.0f;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);
	MotionControl = EAGX_MotionControl::MC_DYNAMICS;
	bTransformRootComponent = false;
}

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		InitializeNative();
	}
	check(HasNative()); /// \todo Consider better error handling that 'check'.
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

bool UAGX_RigidBodyComponent::HasNative()
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
///      done by UAGX_Simulation, they may not happen concurrently.
void UAGX_RigidBodyComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MotionControl != MC_STATIC)
	{
		ReadTransformFromNative();
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
		/// hierarchives, bodies beneath bodies and such, and case must be taken to
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
		UE_LOG(LogAGX, Log, TEXT("Shape added to native object for RigidBody with mass %f."), Mass);
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->AddRigidBody(this);

	UE_LOG(LogAGX, Log, TEXT("Native object created for RigidBody with mass %f."), Mass);
}

void UAGX_RigidBodyComponent::WritePropertiesToNative()
{
	if (!HasNative())
	{
		return;
	}

	NativeBarrier.SetMass(Mass);
	/// \todo Add call to SetInertiaTensorDiagonal here, when it has been implemented.
	NativeBarrier.SetVelocity(Velocity);
	NativeBarrier.SetAngularVelocity(AngularVelocity);
	InitializeMotionControl();
}

void UAGX_RigidBodyComponent::InitializeMotionControl()
{
	NativeBarrier.SetMotionControl(MotionControl);

	if (MotionControl == MC_STATIC && Mobility != EComponentMobility::Static)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"The Rigid Body Component \"%s\" has a RigidBody with Static AGX MotionControl but "
				"Non-Static Unreal Mobility. "
				"Unreal Mobility will automatically be changed to Static this game session, "
				"but should also be "
				"changed manually in the Editor to ensure best performance!"),
			*GetName());

		SetMobility(EComponentMobility::Type::Static);
	}
	else if (MotionControl == MC_DYNAMICS && Mobility != EComponentMobility::Movable)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The Rigid Body Component \"%s\" has a RigidBody with Dynamic AGX MotionControl "
				 "but "
				 "Non-Movable Unreal Mobility. "
				 "Unreal Mobility will automatically be changed to Movable this game session, "
				 "but should also be "
				 "changed manually in the Editor to avoid future problems!"),
			*GetName());

		SetMobility(EComponentMobility::Type::Movable);
	}
}

void UAGX_RigidBodyComponent::ReadTransformFromNative()
{
	check(HasNative());
	const FVector NewLocation = NativeBarrier.GetPosition();
	const FQuat NewRotation = NativeBarrier.GetRotation();
	if (bTransformRootComponent)
	{
		check(GetOwner());
		GetOwner()->SetActorLocationAndRotation(NewLocation, NewRotation);
	}
	else
	{
		const FVector OldLocation = GetComponentLocation();
		const FVector LocationDelta = NewLocation - OldLocation;
		MoveComponent(LocationDelta, NewRotation, false);
	}
}

void UAGX_RigidBodyComponent::WriteTransformToNative()
{
	check(HasNative());
	NativeBarrier.SetPosition(GetComponentLocation());
	NativeBarrier.SetRotation(GetComponentQuat());
}

/// \todo Can use TInlineComponentArray<UAGX_RigidBodyComponent*> here, for performance.
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
