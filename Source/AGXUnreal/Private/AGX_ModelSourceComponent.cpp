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
		UE_LOG(LogAGX, Warning, TEXT("Reading Model Source Component from old serialization."));
		AActor* Owner = GetOwner();
		UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*this);
		UE_LOG(LogAGX, Warning, TEXT("  Owner: %s (%p)"), *GetLabelSafe(Owner), Owner);
		UE_LOG(LogAGX, Warning, TEXT("  Blueprint: %s (%p)"), *GetNameSafe(Blueprint), Blueprint);
		for (const auto& [Name, RenderDataGuid] : StaticMeshComponentToOwningRenderData_DEPRECATED)
		{
			FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult SearchResult =
				FAGX_BlueprintUtilities::GetSCSNodeFromName(*Blueprint, Name, true);
			UActorComponent* Component = SearchResult.FoundNode->ComponentTemplate;
			UE_LOG(
				LogAGX, Warning,
				TEXT("  Found Render Data Static Mesh Component %s for name key %s."),
				*GetNameSafe(Component), *Name);

			USCS_Node* ShapeNode = FAGX_BlueprintUtilities::GetParentSCSNode<UAGX_ShapeComponent>(
				SearchResult.FoundNode);
			UAGX_ShapeComponent* ShapeComponent =
				Cast<UAGX_ShapeComponent>(ShapeNode->ComponentTemplate);
			FGuid ShapeGuid = ShapeComponent->ImportGuid;
			UE_LOG(
				LogAGX, Warning, TEXT("  Found Shape Component %s."), *GetNameSafe(ShapeComponent));

			StaticMeshComponentToOwningShape.Add(Name, ShapeGuid);
		}

		if (!Blueprint->MarkPackageDirty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not mark Blueprint package dirty after Render Data table update in "
					 "Model Source Component."));
		}
	}

}

#undef LOCTEXT_NAMESPACE
