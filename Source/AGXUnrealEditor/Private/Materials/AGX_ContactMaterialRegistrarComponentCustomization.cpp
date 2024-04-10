// Copyright 2024, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarComponentCustomization.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_ContactMaterialRegistrarComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_ContactMaterialRegistrarComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ContactMaterialRegistrarComponentCustomization);
}

void FAGX_ContactMaterialRegistrarComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(FName("Sockets"));
	DetailBuilder.HideCategory(FName("Variable"));
}

#undef LOCTEXT_NAMESPACE
