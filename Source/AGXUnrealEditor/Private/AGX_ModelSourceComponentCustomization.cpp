// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ModelSourceComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_MaterialReplacer.h"
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

	CustomizeMaterialReplacer(ModelSourceComponent);

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

namespace AGX_ModelSourceComponentCustomization_helpers
{
	using FGetMaterial = FString (FAGX_ModelSourceComponentCustomization::*)() const;
	using FSetMaterial = void (FAGX_ModelSourceComponentCustomization::*)(const FAssetData&);
	using FShouldFilterMaterial =
		bool (FAGX_ModelSourceComponentCustomization::*)(const FAssetData&);

	static void CreateMaterialWidget(
		FAGX_ModelSourceComponentCustomization& Customizer, IDetailCategoryBuilder& CategoryBuilder,
		TSharedPtr<FAssetThumbnailPool>& ThumbnailPool, const FText& Label, const FText& ToolTip,
		FGetMaterial GetMaterial, FSetMaterial SetMaterial,
		FShouldFilterMaterial ShouldFilterMaterial)
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
			.ThumbnailPool(ThumbnailPool)
			.OnShouldFilterAsset(&Customizer, ShouldFilterMaterial)
			.ObjectPath(&Customizer, GetMaterial)
			.OnObjectChanged(&Customizer, SetMaterial)
		];
		// clang-format on
	}

	void ExtractRenderMaterials(const UStaticMeshComponent& Mesh, TSet<UObject*>& Materials)
	{
		for (int32 Index = 0; Index < Mesh.GetNumMaterials(); ++Index)
		{
			Materials.Add(Mesh.GetMaterial(Index));
		}
	}

	static TSet<UObject*> ExtractRenderMaterials(UBlueprint& Blueprint)
	{
		TSet<UObject*> Materials;
		for (UStaticMeshComponent* MeshTemplate :
			 FAGX_BlueprintUtilities::GetAllTemplateComponents<UStaticMeshComponent>(Blueprint))
		{
			ExtractRenderMaterials(*MeshTemplate, Materials);
		}
		return Materials;
	}



	static TSet<UObject*> ExtractRenderMaterials(AActor& Actor)
	{
		TArray<UStaticMeshComponent*> Meshes;
		Actor.GetComponents(Meshes);
		TSet<UObject*> Materials;
		Materials.Reserve(Meshes.Num());
		for (UStaticMeshComponent* Mesh : Meshes)
		{
			ExtractRenderMaterials(*Mesh, Materials);
		}
		return Materials;
	}

	static TSet<UObject*> ExtractRenderMaterials(UAGX_ModelSourceComponent* ModelSource)
	{
		// Determine if the Model Source Component is part of a Blueprint or an Actor. This code
		// assumes that Components that are part of a Blueprint, the actual Blueprint and not an
		// instance of a Blueprint, does not have an Owner. This is true as of Unreal Engine 5.3 but
		// is not guaranteed to be true forever.
		UBlueprint* EditBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*ModelSource);
		AActor* Owner = ModelSource->GetOwner();
		if (EditBlueprint == nullptr && Owner == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Material replacing failed because the Model Source Component has neither an "
					 "Owner nor a Blueprint"));
			return TSet<UObject*>();
		}
		else if (EditBlueprint != nullptr)
		{
			return ExtractRenderMaterials(*EditBlueprint);
		}
		else if (Owner != nullptr)
		{
			return ExtractRenderMaterials(*Owner);
		}
		else
		{
			// Should never get here, the above if-else-if chain should cover all cases.
			checkNoEntry();
			return TSet<UObject*>();
		}
	}
};

void FAGX_ModelSourceComponentCustomization::CustomizeMaterialReplacer(
	UAGX_ModelSourceComponent* ModelSource)
{
	using namespace AGX_ModelSourceComponentCustomization_helpers;
	KnownAssets = ExtractRenderMaterials(ModelSource);

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder->EditCategory("Material Replacer");

	TSharedPtr<FAssetThumbnailPool> ThumbnailPool = DetailBuilder->GetThumbnailPool();
	CreateMaterialWidget(
		*this, CategoryBuilder, ThumbnailPool, LOCTEXT("CurrentMaterial", "Current Material"),
		LOCTEXT(
			"CurrentMaterialTooltip",
			"The Material currently set on Static Mesh Components that should be replaced with "
			"another Material."),
		&FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected,
		&FAGX_ModelSourceComponentCustomization::IncludeOnlyUsedMaterials);
	CreateMaterialWidget(
		*this, CategoryBuilder, ThumbnailPool, LOCTEXT("NewMaterial", "New Material"),
		LOCTEXT(
			"NewMaterialTooltip",
			"The Material that should be assigned instead of Current Material."),
		&FAGX_ModelSourceComponentCustomization::GetNewMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected,
		&FAGX_ModelSourceComponentCustomization::IncludeAllMaterials);

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
	return FAGX_MaterialReplacer::GetCurrentPathName();
}

FString FAGX_ModelSourceComponentCustomization::GetNewMaterialPath() const
{
	return FAGX_MaterialReplacer::GetNewPathName();
}

void FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected(const FAssetData& AssetData)
{
	FAGX_MaterialReplacer::SetCurrent(AssetData);
}

void FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected(const FAssetData& AssetData)
{
	FAGX_MaterialReplacer::SetNew(AssetData);
}

bool FAGX_ModelSourceComponentCustomization::IncludeOnlyUsedMaterials(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	const bool HideAsset = !KnownAssets.Contains(Asset);
	return HideAsset;
}

bool FAGX_ModelSourceComponentCustomization::IncludeAllMaterials(const FAssetData& AssetData)
{
	return false;
}

FReply FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked()
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return FReply::Handled();
	};

	if (DetailBuilder == nullptr)
	{
		return Bail(
			TEXT("Material replacing is currenly only supported while there is a Detail Builder."));
	}

	UAGX_ModelSourceComponent* ModelSource =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			*DetailBuilder);
	if (ModelSource == nullptr)
	{
		return Bail(
			TEXT("Material replacing is currenly only supported with a Model Source Component."));
	}

	// Determine if the Model Source Component is part of a Blueprint or an Actor. This code assumes
	// that Components that are part of a Blueprint, the actual Blueprint and not an instance of a
	// Blueprint, does not have an Owner. This is true as of Unreal Engine 5.3 but is not guaranteed
	// to be true forever.
	UBlueprint* EditBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*ModelSource);
	AActor* Owner = ModelSource->GetOwner();
	if (EditBlueprint == nullptr && Owner == nullptr)
	{
		return Bail(
			TEXT("Material replacing failed because the Model Source Component has neither an "
				 "Owner nor a Blueprint"));
	}
	else if (EditBlueprint != nullptr)
	{
		FAGX_MaterialReplacer::ReplaceMaterials(*EditBlueprint);
	}
	else if (Owner != nullptr)
	{
		FAGX_MaterialReplacer::ReplaceMaterials(*Owner);
	}
	else
	{
		// Should never get here, the above if-else-if chain should cover all cases.
		checkNoEntry();
	}

	KnownAssets.Add(FAGX_MaterialReplacer::NewMaterial.Get());

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
