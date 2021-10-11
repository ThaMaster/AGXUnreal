#include "Materials/AGX_ContactMaterialRegistrarActor.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"


#define LOCTEXT_NAMESPACE "AAGX_ContactMaterialRegistrarActor"

AAGX_ContactMaterialRegistrarActor::AAGX_ContactMaterialRegistrarActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent =
		CreateDefaultSubobject<USceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());

	ContactMaterialRegistrarComponent = CreateDefaultSubobject<UAGX_ContactMaterialRegistrarComponent>(
		TEXT("AGX_ContactMaterialRegistrar"));
}

#undef LOCTEXT_NAMESPACE
