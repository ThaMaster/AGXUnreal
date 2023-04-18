// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarActor.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ContactMaterialRegistrarSpriteComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_ContactMaterialRegistrarActor"

AAGX_ContactMaterialRegistrarActor::AAGX_ContactMaterialRegistrarActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<UAGX_ContactMaterialRegistrarSpriteComponent>("SpriteIcon");

	ContactMaterialRegistrarComponent =
		CreateDefaultSubobject<UAGX_ContactMaterialRegistrarComponent>(
			TEXT("AGX_ContactMaterialRegistrar"));
}

#undef LOCTEXT_NAMESPACE
