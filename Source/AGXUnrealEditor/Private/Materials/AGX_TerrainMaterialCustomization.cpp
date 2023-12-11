// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterialCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#include "Materials/AGX_TerrainMaterial.h"
#include "Utilities/AGX_EditorUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainMaterialCustomization"

TSharedRef<IDetailCustomization> FAGX_TerrainMaterialCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_TerrainMaterialCustomization);
}

void FAGX_TerrainMaterialCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_TerrainMaterial* TerrainMaterial =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_TerrainMaterial>(DetailBuilder);

	if (!TerrainMaterial)
		return;

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Material Properties");

	// Hide 'Bulk' properties from the terrain material view. The 'Bulk' properties of the terrain
	// material is specified in FAGX_TerrainMaterialProperties
	DetailBuilder.HideProperty(
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, Bulk)));
}

#undef LOCTEXT_NAMESPACE
