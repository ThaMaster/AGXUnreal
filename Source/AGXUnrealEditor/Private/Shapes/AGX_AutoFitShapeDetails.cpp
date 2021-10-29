#include "Shapes/AGX_AutoFitShapeDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/StaticMesh.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "AGX_AutoFitShapeDetails"

namespace AGX_AutoFitShapeDetals_helpers
{
	bool SupportsAutoFit(UAGX_ShapeComponent* Shape)
	{
		if (Cast<UAGX_BoxShapeComponent>(Shape))
		{
			return true;
		}
		if (Cast<UAGX_CapsuleShapeComponent>(Shape))
		{
			return true;
		}
		if (Cast<UAGX_CylinderShapeComponent>(Shape))
		{
			return true;
		}
		return false;
	}

	AGX_AutoFitShape* ToAutoFitShape(UAGX_ShapeComponent* Shape)
	{
		if (UAGX_BoxShapeComponent* Box = Cast<UAGX_BoxShapeComponent>(Shape))
		{
			return (AGX_AutoFitShape*) Box;
		}
		if (UAGX_CapsuleShapeComponent* Capsule = Cast<UAGX_CapsuleShapeComponent>(Shape))
		{
			return (AGX_AutoFitShape*) Capsule;
		}
		if (UAGX_CylinderShapeComponent* Cylinder = Cast<UAGX_CylinderShapeComponent>(Shape))
		{
			return (AGX_AutoFitShape*) Cylinder;
		}
		return nullptr;
	}

	struct FScopedProperyNotify
	{
		FScopedProperyNotify() = delete;
		FScopedProperyNotify(TArray<TSharedRef<IPropertyHandle>>& InProperties)
			: Properties(InProperties)
		{
			for (TSharedRef<IPropertyHandle> Handle : Properties)
			{
				if (!Handle->IsValidHandle())
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("FScopedProperyNotify was given an invalid propery handle."));
					continue;
				}

				Handle->NotifyPreChange();
			}
		}

		~FScopedProperyNotify()
		{
			for (TSharedRef<IPropertyHandle> Handle : Properties)
			{
				if (Handle->IsValidHandle())
				{
					Handle->NotifyPostChange();
				}
			}
		}

	private:
		const TArray<TSharedRef<IPropertyHandle>>& Properties;
	};
}

FAGX_AutoFitShapeDetails::FAGX_AutoFitShapeDetails(IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	MeshLocations.Add(
		MakeShareable(new FAutoFitMeshLocation("Any Children", EAGX_MeshLocation::AnyChildren)));
	MeshLocations.Add(MakeShareable(
		new FAutoFitMeshLocation("Immediate Children", EAGX_MeshLocation::ImmediateChildren)));
	MeshLocations.Add(MakeShareable(new FAutoFitMeshLocation("Parent", EAGX_MeshLocation::Parent)));
	MeshLocations.Add(MakeShareable(new FAutoFitMeshLocation("Asset", EAGX_MeshLocation::Asset)));
	CurrentlySelectedMeshLocation = MeshLocations[0];
}

void FAGX_AutoFitShapeDetails::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section.
	// The Category we're part of will still be collapsable.
}

void FAGX_AutoFitShapeDetails::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	// clang-format off

	// Add Mesh Location combobox.
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("MeshLocation", "Mesh location"))
		]
		.ValueContent()
		[
			SNew(SComboBox<TSharedPtr<FAutoFitMeshLocation>>)
				//.ContentPadding(2)
				.OptionsSource(&MeshLocations)
				.OnGenerateWidget_Lambda([=](TSharedPtr<FAutoFitMeshLocation> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(*Item->Name));
				})
				.OnSelectionChanged(
					this, &FAGX_AutoFitShapeDetails::OnMeshLocationComboBoxChanged)
				.Content()
				[
					SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return FText::FromString(CurrentlySelectedMeshLocation->Name);
						})
				]
		];

	// Add Static Mesh Asset picker.
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
	.Visibility(TAttribute<EVisibility>(this, &FAGX_AutoFitShapeDetails::GetAssetPickerVisibility))
	.NameContent()
	[
		SNew(STextBlock)
			.Text(LOCTEXT("StaticMeshAsset", "Static Mesh Asset"))
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
		[
			SNew(SObjectPropertyEntryBox)
				.AllowClear(true)
				.AllowedClass(UStaticMesh::StaticClass())
				.ObjectPath(this, &FAGX_AutoFitShapeDetails::GetCurrentAssetPath)
				.OnObjectChanged(this, &FAGX_AutoFitShapeDetails::OnAssetSelected)
				.ThumbnailPool(DetailBuilder.GetPropertyUtilities()->GetThumbnailPool())
		]
	];

	// Add auto-fit button.
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
	.NameContent()
	[
		SNew(STextBlock)
			.Text(LOCTEXT("AutoFitToMesh", "Auto-fit to Mesh"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("AutoFitButtonText", "Auto-fit"))
					.ToolTipText(LOCTEXT(
						"AutoFitButtonTooltip",
						"Auto-fit this Shape to the Static Meshs(es) given by the current Mesh Location."))
					.OnClicked_Lambda([this]() {
						return OnAutoFitButtonClicked();
					})
			]
	];

	// clang-format on
}

bool FAGX_AutoFitShapeDetails::InitiallyCollapsed() const
{
	return false;
}

void FAGX_AutoFitShapeDetails::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
}

FName FAGX_AutoFitShapeDetails::GetName() const
{
	return TEXT("Auto-fit Shape Details");
}

bool FAGX_AutoFitShapeDetails::RequiresTick() const
{
	return false;
}

void FAGX_AutoFitShapeDetails::Tick(float DeltaTime)
{
}

void FAGX_AutoFitShapeDetails::OnAssetSelected(const FAssetData& AssetData)
{
	CurrentlySelectedAsset = AssetData;
}

FString FAGX_AutoFitShapeDetails::GetCurrentAssetPath() const
{
	return CurrentlySelectedAsset.IsValid() ? CurrentlySelectedAsset.ObjectPath.ToString()
											: FString("");
}

FReply FAGX_AutoFitShapeDetails::OnAutoFitButtonClicked()
{
	using namespace AGX_AutoFitShapeDetals_helpers;
	UAGX_ShapeComponent* Shape =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ShapeComponent>(DetailBuilder);

	if (!Shape || !SupportsAutoFit(Shape))
	{
		return FReply::Handled();
	}

	TArray<TSharedRef<IPropertyHandle>> Properties;
	Properties.Add(DetailBuilder.GetProperty(
		USceneComponent::GetRelativeLocationPropertyName(), USceneComponent::StaticClass()));
	Properties.Add(DetailBuilder.GetProperty(
		USceneComponent::GetRelativeRotationPropertyName(), USceneComponent::StaticClass()));

	const FScopedProperyNotify PropertyNotify(Properties);
	const FScopedTransaction Transaction(LOCTEXT("AutoFitUndo", "Undo Auto-fit operation"));

	if (Shape->IsInBlueprint())
	{
		// Logging done in AutoFitInBlueprint.
		/*	return AutoFitInBlueprint(
				AutoFitShapeComponent,
				Cast<UBlueprintGeneratedClass>(AutoFitShapeComponent->GetOuter()));*/
	}

	// Call Modify on children meshes if EAGX_MeshLocation::Children is used, to support
	// undo/redo.
	if (CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AnyChildren ||
		CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::ImmediateChildren)
	{
		const bool Recursive =
			CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AnyChildren;

		for (UStaticMeshComponent* Child :
			 AGX_MeshUtilities::FindChildrenMeshComponents(*Shape, Recursive))
		{
			if (Child == nullptr)
			{
				continue;
			}
			Child->Modify();
		}
	}

	Shape->Modify();
	AGX_AutoFitShape* AutoFitShape = ToAutoFitShape(Shape);
	check(AutoFitShape);
	if (CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AnyChildren ||
		CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::ImmediateChildren)
	{
		const bool Recursive =
			CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AnyChildren;
		AutoFitShape->AutoFitToChildren(
			AGX_MeshUtilities::FindChildrenMeshComponents(*Shape, Recursive),
			Shape->GetWorld(), Shape->GetName());
	}
	else
	{
		AutoFitShape->AutoFit(GetSelectedStaticMeshes(Shape), Shape->GetWorld(), Shape->GetName());
	}

	// Logging done in AutoFit.
	return FReply::Handled();
}

void FAGX_AutoFitShapeDetails::OnMeshLocationComboBoxChanged(
	TSharedPtr<FAutoFitMeshLocation> NewMeshLocation, ESelectInfo::Type InSeletionInfo)
{
	CurrentlySelectedMeshLocation = NewMeshLocation;
}

EVisibility FAGX_AutoFitShapeDetails::GetAssetPickerVisibility() const
{
	return FAGX_EditorUtilities::VisibleIf(
		CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::Asset);
}

UStaticMesh* FAGX_AutoFitShapeDetails::GetSelectedStaticMeshAsset() const
{
	return LoadObject<UStaticMesh>(GetTransientPackage(), *GetCurrentAssetPath());
}

TArray<FAGX_MeshWithTransform> FAGX_AutoFitShapeDetails::GetSelectedStaticMeshes(
	USceneComponent* Shape) const
{
	TArray<FAGX_MeshWithTransform> Meshes;
	if (Shape == nullptr)
	{
		return Meshes;
	}

	switch (CurrentlySelectedMeshLocation->MeshLocation)
	{
		case EAGX_MeshLocation::AnyChildren:
			Meshes = AGX_MeshUtilities::FindChildrenMeshes(*Shape, true);
			break;
		case EAGX_MeshLocation::ImmediateChildren:
			Meshes = AGX_MeshUtilities::FindChildrenMeshes(*Shape, false);
			break;
		case EAGX_MeshLocation::Parent:
			Meshes.Add(AGX_MeshUtilities::FindFirstParentMesh(*Shape));
			break;
		case EAGX_MeshLocation::Asset:
			if (UStaticMesh* MeshAsset = GetSelectedStaticMeshAsset())
			{
				Meshes.Add(FAGX_MeshWithTransform(MeshAsset, Shape->GetComponentTransform()));
			}
			break;
	}

	return Meshes;
}
