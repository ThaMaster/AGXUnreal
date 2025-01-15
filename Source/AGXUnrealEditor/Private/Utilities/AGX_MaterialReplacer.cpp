// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_MaterialReplacer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"
#include "Utilities/AGX_StringUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_MaterialReplacer"

void FAGX_MaterialReplacer::SetCurrent(const FAssetData& AssetData)
{
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

namespace AGX_MaterialReplacer_helpers
{
	TArray<UStaticMeshComponent*> GetStaticMeshTemplates(
		UBlueprint& Blueprint, UClass& BlueprintClass)
	{
		TArray<UStaticMeshComponent*> Templates;
		Templates.Reserve(32);

		// Iterate over the inheritance chain, starting at the given Blueprint.
		for (UBlueprint* Parent = &Blueprint; Parent != nullptr;
			 Parent = FAGX_BlueprintUtilities::GetParent(*Parent))
		{
			// Iterate over all SCS nodes in the Blueprint.
			for (auto Node : Parent->SimpleConstructionScript->GetAllNodes())
			{
				if (Node == nullptr)
					continue;

				// Only interested in Static Mesh Components.
				UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Node->ComponentTemplate);
				if (Mesh == nullptr)
					continue;

				// We only want to update the edit Blueprint, so find the instance that is owned by
				// that Blueprint.
				Mesh = FAGX_ObjectUtilities::GetMatchedInstance(Mesh, &BlueprintClass);
				if (Mesh == nullptr)
					continue;

				Templates.Add(Mesh);
			}
		}

		return Templates;
	}
}

bool FAGX_MaterialReplacer::ReplaceMaterials(UBlueprint& Blueprint)
{
	using namespace AGX_MaterialReplacer_helpers;

	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

	UClass* BlueprintClass = Blueprint.GeneratedClass;
	if (BlueprintClass == nullptr)
	{
		return Bail(
			TEXT("Material replacing failed because the Bluperint doesn't have a generated class. "
				 "It may help to compile the Blueprint first."));
	}

	const UMaterialInterface* const CurrentMat = CurrentMaterial.Get();
	UMaterialInterface* const NewMat = NewMaterial.Get();
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

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	TArray<UStaticMeshComponent*> ChangedBlueprintMeshes;
	ChangedBlueprintMeshes.Reserve(32);

	// Iterate over all Static Mesh SCS nodes. Not only those in the current Blueprint but also all
	// Static Meshes inherited from parent Blueprints.
	for (UStaticMeshComponent* MeshTemplate : GetStaticMeshTemplates(Blueprint, *BlueprintClass))
	{
		for (int32 MaterialIndex = 0; MaterialIndex < MeshTemplate->GetNumMaterials();
			 ++MaterialIndex)
		{
			// Only replace matching Materials.
			if (MeshTemplate->GetMaterial(MaterialIndex) != CurrentMat)
				continue;

			FEditPropertyChain EditPropertyChain;
			EditPropertyChain.AddHead(Property);
			EditPropertyChain.SetActivePropertyNode(Property);

			// Hack: PreEditChange is virtual and overloaded in UObject. However, UActorComponent
			// only overrides one overload of PreEditChange, which hides the other due to C++
			// overloading rules. By casting to the base class, UObject, we include all overloads in
			// that class in the overload set.
			MeshTemplate->UObject::PreEditChange(EditPropertyChain);
			//((UObject*) MeshTemplate)->PreEditChange(EditPropertyChain);

			MeshTemplate->SetMaterial(MaterialIndex, NewMat);

			ChangedBlueprintMeshes.Add(MeshTemplate);

			FPropertyChangedEvent PropertyChangedEvent(Property);

			for (UStaticMeshComponent* Instance :
				 FAGX_ObjectUtilities::GetArchetypeInstances(*MeshTemplate))
			{
				Instance->PreEditChange(Property);
				if (Instance->GetMaterial(MaterialIndex) == CurrentMat)
				{
					Instance->SetMaterial(MaterialIndex, NewMat);
				}
				Instance->PostEditChangeProperty(PropertyChangedEvent);
			}
		}
	}

	for (UStaticMeshComponent* Mesh : ChangedBlueprintMeshes)
	{
		FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet);
		Mesh->PostEditChangeProperty(PropertyChangedEvent);
	}

	return true;
}

bool FAGX_MaterialReplacer::ReplaceMaterials(AActor& Actor)
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

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

	// If the Actor we are modifying is an instance of a Blueprint then we cannot
	const bool bIsBlueprintInstance = [&Actor]()
	{
		USceneComponent* Root = Actor.GetRootComponent();
		if (Root == nullptr)
			return false; // Are we sure? Can a Blueprint be without a root Component?
		UObject* RootArchetype = Root->GetArchetype();
		if (RootArchetype == nullptr)
			return false;
		return !Root->IsInBlueprint() && RootArchetype->IsInBlueprint();
	}();

	auto SetMaterial = [New, Property, &Event, bIsBlueprintInstance](
						   UStaticMeshComponent* Mesh, int32 MaterialIndex)
	{
		Mesh->PreEditChange(Property);
		Mesh->SetMaterial(MaterialIndex, New);
		if (!bIsBlueprintInstance)
			Mesh->PostEditChangeProperty(Event);
	};

	TArray<UStaticMeshComponent*> Meshes;
	Actor.GetComponents(Meshes);

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	for (UStaticMeshComponent* Mesh : Meshes)
	{
		UE_LOG(LogAGX, Warning, TEXT("Found static mesh '%s' at %p."), *GetNameSafe(Mesh), Mesh);
		for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
		{
			UE_LOG(LogAGX, Warning, TEXT("  Checking material index %d."), MaterialIndex);
			UMaterialInterface* Material = Mesh->GetMaterial(MaterialIndex);
			if (Material != Current)
			{
				UE_LOG(LogAGX, Warning, TEXT("  Wrong material, ignoring."));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("  Updating instances."))
			for (UStaticMeshComponent* Instance :
				 FAGX_ObjectUtilities::GetArchetypeInstances(*Mesh))
			{
				UE_LOG(
					LogAGX, Warning, TEXT("    Found instance '%s' at %p."), *GetNameSafe(Instance),
					Instance);
				if (Instance->GetMaterial(MaterialIndex) == Current)
				{
					UE_LOG(LogAGX, Warning, TEXT("    Updating material."));
					SetMaterial(Instance, MaterialIndex);
				}
				else
				{
					UE_LOG(LogAGX, Warning, TEXT("    Wrong material, ignoring."));
				}
			}

			UE_LOG(
				LogAGX, Warning, TEXT("  Setting material on the source mesh: '%s' at %p."),
				*GetNameSafe(Mesh), Mesh);
			SetMaterial(Mesh, MaterialIndex);
		}
	}

	if (bIsBlueprintInstance)
	{
		Actor.PostEditChange();
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
