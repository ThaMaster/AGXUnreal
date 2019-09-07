#include "AGX_RigidBodyComponent.h"

#include "AGX_LogCategory.h"


// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Mass = 10;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);

	// ...
	UE_LOG(LogAGX, Log, TEXT("RigidBody constructor called."))
}

agx::agx_RigidBody* UAGX_RigidBodyComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		InitializeNative();
	}

	return Native;
}

agx::agx_RigidBody* UAGX_RigidBodyComponent::GetNative()
{
	return Native;
}

bool UAGX_RigidBodyComponent::HasNative()
{
	return Native != nullptr;
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

	UE_LOG(LogAGX, Log, TEXT("Searching for geometries."));
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
	Native = agx::allocate(TEXT("agx::RigidBody"));
	agx::call(TEXT("agx::setPosition, velocity, etc"));
}
