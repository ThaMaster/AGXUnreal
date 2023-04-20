// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarActor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ContactMaterialRegistrarSpriteComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_ContactMaterialRegistrarActor"

AAGX_ContactMaterialRegistrarActor::AAGX_ContactMaterialRegistrarActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<UAGX_ContactMaterialRegistrarSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());

	ContactMaterialRegistrarComponent =
		CreateDefaultSubobject<UAGX_ContactMaterialRegistrarComponent>(
			TEXT("AGX_ContactMaterialRegistrar"));
}

void AAGX_ContactMaterialRegistrarActor::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);

	if (RootComponent == nullptr &&
		ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainCGDisablerCMRegistrarViewporIcons))
	{
		SetRootComponent(NewObject<UAGX_ContactMaterialRegistrarSpriteComponent>(
			this, USceneComponent::GetDefaultSceneRootVariableName()));
		RootComponent->SetFlags(RF_Transactional);
		AddInstanceComponent(RootComponent);
		// We should not register the Component here because it is too early. The Component will be
		// registered automatically by this Actor.
	}
}

#undef LOCTEXT_NAMESPACE
