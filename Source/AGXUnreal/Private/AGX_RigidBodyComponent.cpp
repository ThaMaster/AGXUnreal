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
	Mass = 10;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);
	UE_LOG(LogAGX, Log, TEXT("RigidBodyComponent is being ticked at %d."), (int)PrimaryComponentTick.TickGroup);
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

// Called every frame
void UAGX_RigidBodyComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only printing tick output once to avoid excessive spam in the output window.
	static bool HavePrinted = false;
	if (!HavePrinted)
	{
		HavePrinted = true;
		agx::call(TEXT("TickComponent for RigidBody."));
		/// \todo Figure out how to do this in the pre-physics callback.
//		agx::call(TEXT("agx::RigidBody::setPosition, velocity, etc"));

		/// \todo Figure out how to do this in the pos-physics callback.
//		agx::call(TEXT("agx::RigidBody::getPosition, velocity, etc"));
	}
	UE_LOG(LogAGX, Log, TEXT("Tick for body. New height: %f"), NativeBarrier.GetPosition().Z);
}


// Called when the game starts
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
