#include "AGX_RigidBodyComponent.h"
#include "AGX_ShapeComponent.h"
#include "AGX_LogCategory.h"
#include "AGXDynamicsMockup.h"

#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Mass = 10;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);

	// ...
	UE_LOG(LogAGX, Log, TEXT("RigidBody instance created."))
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
	return &NativeBarrier;
}

bool UAGX_RigidBodyComponent::HasNative()
{
	return NativeBarrier.HasNative();
}

// Called when the game starts
void UAGX_RigidBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
	UE_LOG(LogAGX, Log, TEXT("RigidBody with mass %f ready to simulate."), Mass);
}

// Called every frame
void UAGX_RigidBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only printing tick output once to avoid excessive spam in the output window.
	static bool HavePrinted = false;
	if (!HavePrinted)
	{
		HavePrinted = true;
		/// \todo Figure out how to do this in the pre-physics callback.
		agx::call(TEXT("agx::RigidBody::setPosition, velocity, etc"));

		/// \todo Figure out how to do this in the pos-physics callback.
		agx::call(TEXT("agx::RigidBody::getPosition, velocity, etc"));
	}
}

void UAGX_RigidBodyComponent::InitializeNative()
{
	NativeBarrier.AllocateNative();
	agx::call(TEXT("agx::setPosition, velocity, etc"));

	UE_LOG(LogAGX, Log, TEXT("Searching for geometries."));
	// TODO: Restore this shape creation loop.
	#if 0
	TArray<UActorComponent*> shapes = GetOwner()->GetComponentsByClass(UAGX_ShapeComponent::StaticClass());

	for (UActorComponent* component : shapes)
	{
		UAGX_ShapeComponent* ShapeComponent = nullptr;  // Cast<UAGX_ShapeComponent>(component);
		agx::agxCollide_Shape* NativeShape = ShapeComponent->GetOrCreateNative();
		agx::call(TEXT("Native->add(NativeShape);"));
//		Native->add();
	}
	#endif
}
