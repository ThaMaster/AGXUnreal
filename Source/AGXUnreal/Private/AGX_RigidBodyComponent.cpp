#include "AGX_RigidBodyComponent.h"
#include "AGX_ShapeComponent.h"
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

	Mass = 10;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);
	MotionControl = EAGX_MotionControl::MC_DYNAMICS;

	UE_LOG(LogAGX, Log, TEXT("RigidBodyComponent is being ticked at %d."), (int) PrimaryComponentTick.TickGroup);
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

	/// \todo Figure out how to do this in the pos-physics callback.
	agx::call(TEXT("agx::RigidBody::getPosition, velocity, etc"));
	FVector NewLocation = NativeBarrier.GetPosition(GetWorld());
	GetOwner()->SetActorLocation(NewLocation);

	UE_LOG(LogAGX, Log, TEXT("Tick for body. New height: %f"), NativeBarrier.GetPosition(GetWorld()).Z);
}

void UAGX_RigidBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
	NativeBarrier.SetMass(Mass);
	UE_LOG(LogAGX, Log, TEXT("BeginPlay for RigidBody with mass %f."), Mass);

	UGameInstance* Game = GetOwner()->GetGameInstance();
	UAGX_Simulation* Simulation = Game->GetSubsystem<UAGX_Simulation>();
	Simulation->AddRigidBody(this);
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
	NativeBarrier.SetMass(Mass);
	NativeBarrier.SetMotionControl(MotionControl);
	NativeBarrier.SetPosition(GetOwner()->GetActorLocation(), GetWorld());

	TArray<UActorComponent*> Shapes = GetOwner()->GetComponentsByClass(UAGX_ShapeComponent::StaticClass());
	for (UActorComponent* Component : Shapes)
	{
		UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Component);
		FShapeBarrier* NativeShape = Shape->GetOrCreateNative();
		NativeBarrier.AddShape(NativeShape);
		UE_LOG(LogAGX, Log, TEXT("Shape added to native object for RigidBody with mass %f."), Mass);
	}
	UE_LOG(LogAGX, Log, TEXT("Native object created for RigidBody with mass %f."), Mass);
}
