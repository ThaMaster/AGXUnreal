// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ModelSourceComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_ModelSourceComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_ModelSourceComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ModelSourceComponentCustomization);
}

void FAGX_ModelSourceComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_ModelSourceComponent* ModelSourceComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			InDetailBuilder);
	if (!ModelSourceComponent)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("AGX Synchronize Model");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SynchronizeModelButtonText", "Synchronize Model"))
			.ToolTipText(LOCTEXT(
				"SynchronizeModelTooltip",
				"Synchronizes the model against the source file of the original "
				"import and updates the Components and Assets to match said source file."))
			.OnClicked(this, &FAGX_ModelSourceComponentCustomization::OnSynchronizeModelButtonClicked)
		]
	];
	// clang-format on

	CustomizeMaterialReplacer();

	InDetailBuilder.HideCategory(FName("AGX Synchronize Model Info"));
	InDetailBuilder.HideCategory(FName("Variable"));
	InDetailBuilder.HideCategory(FName("Sockets"));
	InDetailBuilder.HideCategory(FName("Tags"));
	InDetailBuilder.HideCategory(FName("ComponentTick"));
	InDetailBuilder.HideCategory(FName("ComponentReplication"));
	InDetailBuilder.HideCategory(FName("Activation"));
	InDetailBuilder.HideCategory(FName("Cooking"));
	InDetailBuilder.HideCategory(FName("Events"));
	InDetailBuilder.HideCategory(FName("AssetUserData"));
	InDetailBuilder.HideCategory(FName("Collision"));
	InDetailBuilder.HideCategory(FName("Replication"));
}

namespace AGX_ModelSourceComponentCustomization_helpers
{
	UBlueprint* GetBlueprint(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_ModelSourceComponent* ModelSourceComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
				DetailBuilder);
		if (ModelSourceComponent == nullptr)
		{
			return nullptr;
		}

		if (!ModelSourceComponent->IsInBlueprint())
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Model synchronization is only supported when in a Blueprint.");
			return nullptr;
		}

		if (auto Bp = Cast<UBlueprintGeneratedClass>(ModelSourceComponent->GetOuter()))
		{
			if (Bp->SimpleConstructionScript != nullptr)
			{
				return Bp->SimpleConstructionScript->GetBlueprint();
			}
		}

		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Unable to get the Blueprint from the AGX Model Source Component. Model "
			"synchronization will not be possible.");
		return nullptr;
	}
}

FReply FAGX_ModelSourceComponentCustomization::OnSynchronizeModelButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	using namespace AGX_ModelSourceComponentCustomization_helpers;
	UBlueprint* Blueprint = GetBlueprint(*DetailBuilder);
	if (Blueprint == nullptr)
	{
		// Logging done in GetBlueprint.
		return FReply::Handled();
	}

	FAGX_EditorUtilities::SynchronizeModel(*Blueprint);

	// Any logging is done in SynchronizeModel.
	return FReply::Handled();
}

struct FAGX_ModelSourceComponentCustomization_helper
{
	using FGetMaterial = FString (FAGX_ModelSourceComponentCustomization::*)() const;
	using FSetMaterial = void (FAGX_ModelSourceComponentCustomization::*)(const FAssetData&);

	static void CreateMaterialWidget(
		FAGX_ModelSourceComponentCustomization& Customizer, IDetailCategoryBuilder& CategoryBuilder,
		const FText& Label, const FText& ToolTip, FGetMaterial GetMaterial,
		FSetMaterial SetMaterial)
	{
		// clang-format off
		CategoryBuilder.AddCustomRow(LOCTEXT("ReplaceMaterial", "Replace Material"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(Label)
			.ToolTipText(ToolTip)
		]
		.ValueContent()
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UMaterialInterface::StaticClass())
			.ObjectPath(&Customizer, GetMaterial)
			.OnObjectChanged(&Customizer, SetMaterial)
		];
		// clang-format on
	}
};

void FAGX_ModelSourceComponentCustomization::CustomizeMaterialReplacer()
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder->EditCategory("Material Replacer");

	FAGX_ModelSourceComponentCustomization_helper::CreateMaterialWidget(
		*this, CategoryBuilder, LOCTEXT("CurrentMaterial", "Current Material"),
		LOCTEXT(
			"CurrentMaterialTooltip",
			"The Material currently set on Static Mesh Components that should be replaced with "
			"another Material."),
		&FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected);
	FAGX_ModelSourceComponentCustomization_helper::CreateMaterialWidget(
		*this, CategoryBuilder, LOCTEXT("NewMaterial", "New Material"),
		LOCTEXT(
			"NewMaterialTooltip",
			"The Material that should be assigned instead of Current Material."),
		&FAGX_ModelSourceComponentCustomization::GetNewMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected);

	// clang-format off
	CategoryBuilder.AddCustomRow(LOCTEXT("ReplaceMaterialButton", "Replace Material Button"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ReplaceMaterialsButton", "Replace Materials"))
			.ToolTipText(LOCTEXT(
				"ReplaceMaterialsButtonTooltip",
				"Replace all occurrences of Current Material with New Material in this Blueprint"))
			.OnClicked(this, &FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked)
		]
	];
	// clang-format on
}

FString FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath() const
{
	return CurrentMaterial.IsValid() ? CurrentMaterial->GetPathName() : FString();
}

FString FAGX_ModelSourceComponentCustomization::GetNewMaterialPath() const
{
	return NewMaterial.IsValid() ? NewMaterial->GetPathName() : FString();
}

void FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected(const FAssetData& AssetData)
{
	CurrentMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
	if (CurrentMaterial.IsValid())
	{
		UE_LOG(LogAGX, Warning, TEXT("Selected Current Material: %s"), *CurrentMaterial->GetName());
	}
}

void FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected(const FAssetData& AssetData)
{
	NewMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
	if (NewMaterial.IsValid())
	{
		UE_LOG(LogAGX, Warning, TEXT("Selected New Material: %s"), *NewMaterial->GetName());
	}
}

FReply FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked()
{
	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	// TODO Remove trace log.
	UE_LOG(
		LogAGX, Warning, TEXT("Replacing all instances of %s with %s."),
		*GetNameSafe(CurrentMaterial.Get()), *GetNameSafe(NewMaterial.Get()));

	if (DetailBuilder == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			TEXT("Material replacing is currenly only supported while there is a Detail Builder."),
			SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	// It is OK for the Materials to not be set, means that all uses of the default material should
	// be replaced, or that the selected material should be cleared to the default material.

	UAGX_ModelSourceComponent* ModelSource =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			*DetailBuilder);
	if (ModelSource == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			TEXT("Material replacing is currenly only supported with a Model Source Component."),
			SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT("Edited Model Source Component is '%s', the outer is '%s' of type '%s'."),
		*GetNameSafe(ModelSource), *GetNameSafe(ModelSource->GetOuter()),
		*GetNameSafe(ModelSource->GetOuter()->GetClass()));

	UBlueprint* EditBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*ModelSource);
#if 0
	// TODO I think this is just wrong. Consider removing.
	if (EditBlueprint == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not get Blueprint from selected Model Source Component, trying the "
				 "archetype instance."));
		UAGX_ModelSourceComponent* Archetype =
			Cast<UAGX_ModelSourceComponent>(ModelSource->GetArchetype());
		if (Archetype != nullptr)
		{
			EditBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*Archetype);
		}
	}
#endif
	if (EditBlueprint == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			TEXT("Material replacing is currenly only supported from the Blueprint Editor."),
			SNotificationItem::CS_Fail);
		return FReply::Handled();
	}


	UClass* EditedBlueprintClass = EditBlueprint->GeneratedClass;

	UMaterialInterface* Current = CurrentMaterial.Get();
	UMaterialInterface* New = NewMaterial.Get();

	FProperty* Property =
		UStaticMeshComponent::StaticClass()->FindPropertyByName("OverrideMaterials");
	FProperty* GptProperty = FindFieldChecked<FProperty>(UStaticMeshComponent::StaticClass(), TEXT("OverrideMaterials"));
	UE_LOG(LogAGX, Warning, TEXT("\nFindPropertyByName: %p\nFindFieldChecked:   %p"), Property, GptProperty);

	TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder->GetProperty(
		GET_MEMBER_NAME_CHECKED(UStaticMeshComponent, OverrideMaterials), UStaticMeshComponent::StaticClass());
	PropertyHandle->NotifyPreChange();

	int32 NumReplaced {0};

	// Iterate over all SCS nodes. Not only those in the child Blueprint but also all from parent
	// Blueprints.
	for (UBlueprint* BlueprintIt = EditBlueprint; BlueprintIt != nullptr;
		 BlueprintIt = FAGX_BlueprintUtilities::GetParent(*BlueprintIt))
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Iterating over SCS Nodes in '%s'."), *GetNameSafe(BlueprintIt));

		// Iterate over all SCS nodes in the current Blueprint.
		for (auto Node : BlueprintIt->SimpleConstructionScript->GetAllNodes())
		{
			UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Node->ComponentTemplate);
			if (Mesh == nullptr)
				continue;

			UE_LOG(
				LogAGX, Warning, TEXT("Considering Static Mesh Component '%s'"),
				*GetNameSafe(Mesh));

			// We only want to update the edit Blueprint, so find the instance that is owned by
			// the that Blueprint.
			Mesh = FAGX_ObjectUtilities::GetMatchedInstance(Mesh, EditedBlueprintClass);
			if (Mesh == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("  Did not have any instance owned by the edited Blueprint. Skipping."));
				continue;
			}

			UE_LOG(
				LogAGX, Warning, TEXT("  Considering Materials in '%s', part of '%s'."),
				*GetNameSafe(Mesh), *GetNameSafe(FAGX_BlueprintUtilities::GetBlueprintFrom(*Mesh)));

			for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
			{
				UMaterialInterface* Material = Mesh->GetMaterial(MaterialIndex);
				UE_LOG(LogAGX, Warning, TEXT("  Found Material '%s'."), *GetNameSafe(Material));
				if (Material != Current)
				{
					UE_LOG(LogAGX, Warning, TEXT("    Wrong material, skipping."));
					continue;
				}

				UE_LOG(LogAGX, Warning, TEXT("    Correct material, replacing."));

				for (UStaticMeshComponent* Instance :
					 FAGX_ObjectUtilities::GetArchetypeInstances(*Mesh))
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("      Considering updating instance '%s', has Material '%s'."),
						*GetNameSafe(Instance), *GetNameSafe(Instance->GetMaterial(MaterialIndex)));
					if (Instance->GetMaterial(MaterialIndex) == Current)
					{
						UE_LOG(
							LogAGX, Warning,
							TEXT("        That is the one we want to replace. Replacing."));
						Instance->PreEditChange(Property);
						Instance->Modify();
						Instance->SetMaterial(MaterialIndex, New);
						FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);
						Instance->PostEditChangeProperty(Event);
						++NumReplaced;
					}
				}

				Mesh->PreEditChange(Property);
				Mesh->Modify();
				Mesh->SetMaterial(MaterialIndex, New);
				FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);
				Mesh->PostEditChangeProperty(Event);
				++NumReplaced;
			}
		}

		if (NumReplaced > 0)
		{
			EditBlueprint->Modify();
		}

		PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);

		UE_LOG(LogAGX, Warning, TEXT("Num materials replaced: %d."), NumReplaced);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
