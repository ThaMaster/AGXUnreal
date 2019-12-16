#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "AGXDynamicsMockup.h"

#include "GameFramework/Actor.h"

#include "Engine/GameInstance.h"

// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	UE_LOG(LogAGX, Log, TEXT("RigidBody instance created."));
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	Mass = 1.0f;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);
	MotionControl = EAGX_MotionControl::MC_DYNAMICS;

	UE_LOG(
		LogAGX, Log, TEXT("RigidBodyComponent is being ticked at %d."),
		(int) PrimaryComponentTick.TickGroup);
}

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		InitializeNative();
	}
	return &NativeBarrier;
}

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetNative()
{
	if (!HasNative())
	{
		/// \todo Should we return nullptr here, or should we return a pointer
		///       to the existing but empty Barrier object?
		///       Update function documentation in .h if changed.
		return nullptr;
	}
	return &NativeBarrier;
}

bool UAGX_RigidBodyComponent::HasNative()
{
	return NativeBarrier.HasNative();
}

/// \todo Split the UAGX_RigidBodyComponent::TickComponent callback into two
///       parts. One in PrePhysics that reads the Unreal state to AGX Dynamics
///       and one in PostPhysics that read the AGX Dynamics state to Unreal.
///       Read about tick splitting under Advanced Ticking Functionality at
///       https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Actors/Ticking/index.html
///
///      Take care to synchronize this with the actual AGX Dynamics stepping
///      done by UAGX_Simulation.
void UAGX_RigidBodyComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if 0
	/// \todo Figure out how to do this in the pre-physics callback, but not at
	///       the same time as the stepping done in UAGX_Simulation.
	agx::call(TEXT("agx::RigidBody::setPosition, velocity, etc"));
#endif

	/// \todo Figure out how to do this in the post-physics callback.
	if (MotionControl != MC_STATIC)
		UpdateActorTransformsFromNative();

#if 0
	UE_LOG(LogAGX, Log, TEXT("Body placement updated."));
#endif
}

void UAGX_RigidBodyComponent::UpdateNativeProperties()
{
	if (HasNative())
	{
		NativeBarrier.SetMass(Mass);
		NativeBarrier.SetVelocity(Velocity);
		NativeBarrier.SetAngularVelocity(AngularVelocity);
		InitializeMotionControl();
	}
}

UAGX_RigidBodyComponent* UAGX_RigidBodyComponent::GetFromActor(const AActor* Actor)
{
	if (!Actor)
		return nullptr;

	return Actor->FindComponentByClass<UAGX_RigidBodyComponent>();
}

void UAGX_RigidBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
	UE_LOG(LogAGX, Log, TEXT("BeginPlay for RigidBody with mass %f."), Mass);
}

void UAGX_RigidBodyComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	NativeBarrier.ReleaseNative();
	UE_LOG(LogAGX, Log, TEXT("EndPlay for RigidBody with mass %f."), Mass);
}

void UAGX_RigidBodyComponent::InitializeNative()
{
	NativeBarrier.AllocateNative();
	agx::call(TEXT("agx::setPosition, velocity, etc"));
	UpdateNativeProperties();
	UpdateNativeTransformsFromActor();

	TArray<UActorComponent*> Shapes =
		GetOwner()->GetComponentsByClass(UAGX_ShapeComponent::StaticClass());
	for (UActorComponent* Component : Shapes)
	{
		UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Component);
		FShapeBarrier* NativeShape = Shape->GetOrCreateNative();
		/// \todo Should not crash on this. HeightField easy to get wrong.
		check(NativeShape && NativeShape->HasNative());
		NativeBarrier.AddShape(NativeShape);
		UE_LOG(LogAGX, Log, TEXT("Shape added to native object for RigidBody with mass %f."), Mass);
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(GetOwner());
	Simulation->AddRigidBody(this);

	UE_LOG(LogAGX, Log, TEXT("Native object created for RigidBody with mass %f."), Mass);
}

void UAGX_RigidBodyComponent::InitializeMotionControl()
{
	NativeBarrier.SetMotionControl(MotionControl);

	if (USceneComponent* RootComponent = GetOwner()->GetRootComponent())
	{
		if (MotionControl == MC_STATIC && !GetOwner()->IsRootComponentStatic())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("The Actor \"%s\" has a RigidBody with Static AGX MotionControl but "
					 "Non-Static Unreal Mobility. "
					 "Unreal Mobility will automatically be changed to Static this game session, "
					 "but should also be "
					 "changed manually in the Editor to ensure best performance!"),
				*GetOwner()->GetName());

			RootComponent->SetMobility(EComponentMobility::Type::Static);
		}
		else if (MotionControl == MC_DYNAMICS && !GetOwner()->IsRootComponentMovable())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("The Actor \"%s\" has a RigidBody with Dynamic AGX MotionControl but "
					 "Non-Movable Unreal Mobility. "
					 "Unreal Mobility will automatically be changed to Movable this game session, "
					 "but should also be "
					 "changed manually in the Editor to avoid future problems!"),
				*GetOwner()->GetName());

			RootComponent->SetMobility(EComponentMobility::Type::Movable);
		}
	}
}

void UAGX_RigidBodyComponent::UpdateActorTransformsFromNative()
{
	check(HasNative());
	check(GetOwner());

	FVector NewLocation = NativeBarrier.GetPosition();
	GetOwner()->SetActorLocation(NewLocation);

	FQuat NewRotation = NativeBarrier.GetRotation();
	GetOwner()->SetActorRotation(NewRotation);
}

void UAGX_RigidBodyComponent::UpdateNativeTransformsFromActor()
{
	check(HasNative());
	NativeBarrier.SetPosition(GetOwner()->GetActorLocation());
	NativeBarrier.SetRotation(GetOwner()->GetActorQuat());
}
