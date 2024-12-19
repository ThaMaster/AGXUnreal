// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ModelSourceComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Serialization/Archive.h"

#define LOCTEXT_NAMESPACE "UAGX_ModelSourceComponent"

void UAGX_ModelSourceComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);

	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (Archive.IsLoading() &&
		Archive.CustomVer(FAGX_CustomVersion::GUID) < FAGX_CustomVersion::RenderDataPerShape)
	{
		UpgradeRenderDataTableFromRenderDataUuidToShapeUuid();
	}
}

/**
 * To support importing AGX Dynamics archives where multiple Shapes share the same Render Data
 * we changed a table to contain Shape GUIDs instead of Render Data GUIDs. When opening old
 * imported model Blueprints the old table entries must be converted into new table entries.
 * This is done by finding the first parent Shape Component of each Static Mesh Component that
 * renders Render Data and use that Shape Component's Import GUID instead of the Render Data's
 * GUID.
 */
void UAGX_ModelSourceComponent::UpgradeRenderDataTableFromRenderDataUuidToShapeUuid()
{
	// For now, we only do the table translation for Blueprints. Is there any case where the
	// translation must be done for a regular Actor? Any other case we should consider?
	UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*this);
	if (Blueprint == nullptr)
		return;

	// Migrate each entry in the old table to the new table.
	for (const auto& [StaticMeshName, RenderDataGuid] :
		 StaticMeshComponentToOwningRenderData_DEPRECATED)
	{
		// Find the Static Mesh Component that renders the Render Data.
		//
		// This works in Unreal Engine 5.3, but a change in how Blueprints are loaded in Unreal
		// Engine 5.4 made it so that the SCS Node tree has not yet been constructed when Serialize
		// is called on the Model Source Component, which means that we cannot find the Render Data
		// Static Mesh Component SCS Node.
		//
		// A workaround is to call this function manually and explicitly at a later time, when the
		// Blueprint has been fully loaded. For model synchronization this is done by the
		// SCS Node Collection constructor.
		USCS_Node* RenderDataMeshNode =
			FAGX_BlueprintUtilities::GetSCSNodeFromName(*Blueprint, StaticMeshName, true).FoundNode;
		if (RenderDataMeshNode == nullptr)
			continue;

		// Find the Shape Component that owns the Render Data.
		USCS_Node* ShapeNode =
			FAGX_BlueprintUtilities::GetParentSCSNode<UAGX_ShapeComponent>(RenderDataMeshNode);
		if (ShapeNode == nullptr)
			continue;
		UAGX_ShapeComponent* ShapeComponent =
			Cast<UAGX_ShapeComponent>(ShapeNode->ComponentTemplate);
		if (ShapeComponent == nullptr)
			continue;

		// Create entry in the new table.
		FGuid ShapeGuid = ShapeComponent->ImportGuid;
		StaticMeshComponentToOwningShape.Add(StaticMeshName, ShapeGuid);
	}

	// We would like to save the Blueprint with the updated table and to do that we need to mark
	// it dirty. However, Unreal does not allow marking a package dirty during loading.
	// So the old table will linger until the Blueprint is changed through some other means.
	// Worse, the Blueprint is the hidden parent Blueprint that the user should not modify, so
	// it likely won't be modified until the user does a model synchronization.
#if 0
		Blueprint->MarkPackageDirty();
#endif
}

const TMap<FString, FGuid> UAGX_ModelSourceComponent::GetDeprecatedRenderDataTable() const
{
	return StaticMeshComponentToOwningRenderData_DEPRECATED;
}

#undef LOCTEXT_NAMESPACE
