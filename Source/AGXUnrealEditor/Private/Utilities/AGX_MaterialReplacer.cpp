// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_MaterialReplacer.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"

#define LOCTEXT_NAMESPACE "FAGX_MaterialReplacer"

void FAGX_MaterialReplacer::SetCurrent(const FAssetData& AssetData){
	CurrentMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

void FAGX_MaterialReplacer::SetNew(const FAssetData& AssetData)
{
	NewMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

FString FAGX_MaterialReplacer::GetCurrentPathName()
{
	return CurrentMaterial.IsValid() ? CurrentMaterial->GetPathName() : FString();
}

FString FAGX_MaterialReplacer::GetNewPathName()
{
	return NewMaterial.IsValid() ? NewMaterial->GetPathName() : FString();
}

bool FAGX_MaterialReplacer::ReplaceMaterials(UBlueprint* EditBlueprint)
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

	UClass* EditedBlueprintClass = EditBlueprint->GeneratedClass;
	if (EditedBlueprintClass == nullptr)
	{
		return Bail(
			TEXT("Material replacing failed because the Bluperint doesn't have a generated class. "
				 "It may help to compile the Blueprint first."));
	}

	const UMaterialInterface* const Current = CurrentMaterial.Get();
	UMaterialInterface* const New = NewMaterial.Get();
	// It is OK for the Materials to not be set, i.e. be nullptr. It means that all uses of the
	// default material should be replaced, or that the selected current material should be cleared
	// to the default material.

	FProperty* const Property = UStaticMeshComponent::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(UStaticMeshComponent, OverrideMaterials));
	if (Property == nullptr)
	{
		return Bail(
			TEXT("Material replacement could not be completed because Static Mesh Component does "
				 "not have an Override Materials property."));
	}

	FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);

	auto SetMaterial = [New, Property, &Event](UStaticMeshComponent* Mesh, int32 MaterialIndex)
	{
		Mesh->PreEditChange(Property);
		Mesh->SetMaterial(MaterialIndex, New);
		Mesh->PostEditChangeProperty(Event);
	};

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	// Iterate over all SCS nodes. Not only those in the child Blueprint but also all from parent
	// Blueprints.
	for (UBlueprint* BlueprintIt = EditBlueprint; BlueprintIt != nullptr;
		 BlueprintIt = FAGX_BlueprintUtilities::GetParent(*BlueprintIt))
	{
		// Iterate over all SCS nodes in the current Blueprint.
		for (auto Node : BlueprintIt->SimpleConstructionScript->GetAllNodes())
		{
			UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Node->ComponentTemplate);
			if (Mesh == nullptr)
				continue;

			// We only want to update the edit Blueprint, so find the instance that is owned by
			// that Blueprint.
			Mesh = FAGX_ObjectUtilities::GetMatchedInstance(Mesh, EditedBlueprintClass);
			if (Mesh == nullptr)
				continue;

			for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
			{
				UMaterialInterface* Material = Mesh->GetMaterial(MaterialIndex);
				if (Material != Current)
					continue;

				for (UStaticMeshComponent* Instance :
					 FAGX_ObjectUtilities::GetArchetypeInstances(*Mesh))
				{
					if (Instance->GetMaterial(MaterialIndex) == Current)
					{
						SetMaterial(Instance, MaterialIndex);
					}
				}

				SetMaterial(Mesh, MaterialIndex);
			}
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
