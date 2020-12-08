#include "Tires/AGX_TwoBodyTireActor.h"

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "Engine/EngineTypes.h"

AAGX_TwoBodyTireActor::AAGX_TwoBodyTireActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root =
		CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;

	TireRigidBodyComponent =
		CreateDefaultSubobject<UAGX_RigidBodyComponent>(TEXT("TireRigidBodyComponent"));
	TireRigidBodyComponent->SetupAttachment(RootComponent);

	HubRigidBodyComponent =
		CreateDefaultSubobject<UAGX_RigidBodyComponent>(TEXT("HubRigidBodyComponent"));
	HubRigidBodyComponent->SetupAttachment(RootComponent);

	TwoBodyTireComponent =
		CreateDefaultSubobject<UAGX_TwoBodyTireComponent>(TEXT("TwoBodyTireComponent"));

	TwoBodyTireComponent->TireRigidBody.OwningActor = this;
	TwoBodyTireComponent->TireRigidBody.BodyName = TireRigidBodyComponent->GetFName();
	TwoBodyTireComponent->TireRigidBody.CacheCurrentRigidBody();

	TwoBodyTireComponent->HubRigidBody.OwningActor = this;
	TwoBodyTireComponent->HubRigidBody.BodyName = HubRigidBodyComponent->GetFName();
	TwoBodyTireComponent->HubRigidBody.CacheCurrentRigidBody();
}
