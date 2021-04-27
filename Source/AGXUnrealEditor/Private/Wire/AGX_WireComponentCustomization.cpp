#include "AGX_WireComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireComponentVisualizer.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "ScopedTransaction.h"
#include "UnrealEd.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_WireComponentCustomization"

/**
 * A Slate builder that produces the widgets exposing the properties for a single selected wire
 * node.
 */
class FWireNodeDetails : public IDetailCustomNodeBuilder, public TSharedFromThis<FWireNodeDetails>
{
public:
	FWireNodeDetails(UAGX_WireComponent* InWire);

	//~ Begin IDetailCustomNodeBuilder interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder interface

	/** Copy node properties from the selected node, if any, into staging TOptional members. */
	void UpdateValues();

	/** Widgets with this visibility is only shown when a wire node is selected. */
	EVisibility WithSelection() const;

	/** Widgets with this visibility is only shown when not wire node is selected. */
	EVisibility WithoutSelection() const;

private:
	TOptional<float> GetLocationX() const;
	TOptional<float> GetLocationY() const;
	TOptional<float> GetLocationZ() const;

	/** Callback called when the Location widget is edited. */
	void OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis);

	/** Called for each entry in WireNodeTypes, to generate the options for the combo box. */
	TSharedRef<SWidget> OnGenerateComboWidget(TSharedPtr<FString> InComboString);

	/** Called to generate the default (non-opened) view of the node type combo box. */
	FText GetNodeType() const;

	void OnNodeTypeChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

private:
	/** The currently selected Wire Component. This can change. */
	UAGX_WireComponent* Wire;

	/**
	 * The currently active Wire Component Visualizer. Not sure how this works with multiple Detail
	 * Panels, selections, and Viewports.
	 */
	FAGX_WireComponentVisualizer* WireVisualizer;

	/**
	 * The index of the currently selected node in the Wire Component Visualizer. This is the node
	 * whose properties is shown in the widgets, and edits made in the widgets will be applied to
	 * this node.
	 */
	int32 SelectedNodeIndex;

	// Backing storage for the selected node's location.
	TOptional<float> LocationX;
	TOptional<float> LocationY;
	TOptional<float> LocationZ;

	// Backing storage for the selected node's type.
	TOptional<EWireNodeType> NodeType;

	// Backing storage for the node type combo box.
	TArray<TSharedPtr<FString>> WireNodeTypes;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> NodeTypeComboBox;

	// Don't know what this is.
	FSimpleDelegate OnRegenerateChildren;
};

FWireNodeDetails::FWireNodeDetails(UAGX_WireComponent* InWire)
	: Wire(InWire)
{
	check(Wire);
	FComponentVisualizer* Visualizer = GUnrealEd->FindComponentVisualizer(Wire->GetClass()).Get();
	WireVisualizer = (FAGX_WireComponentVisualizer*) Visualizer;
	check(WireVisualizer);

	UEnum* NodeTypesEnum = StaticEnum<EWireNodeType>();
	check(NodeTypesEnum);
	for (int32 EnumIndex = 0; EnumIndex < NodeTypesEnum->NumEnums() - 1; ++EnumIndex)
	{
		WireNodeTypes.Add(
			MakeShareable(new FString(NodeTypesEnum->GetNameStringByIndex(EnumIndex))));
	}
}

void FWireNodeDetails::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
}

void FWireNodeDetails::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	//clang-format off

	ChildrenBuilder.AddCustomRow(LOCTEXT("NoSelection", "No Selection"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithoutSelection))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("NoNodeSelection", "No node is selected"))
	];

	ChildrenBuilder.AddCustomRow(LOCTEXT("Title", "Title"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithSelection))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SelectedNode", "Properties of the currently selected node"))
	];

	ChildrenBuilder.AddCustomRow(LOCTEXT("Location", "Location"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithSelection))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Location", "Location"))
	]
	.ValueContent()
	[
		SNew(SVectorInputBox)
		.X(this, &FWireNodeDetails::GetLocationX)
		.Y(this, &FWireNodeDetails::GetLocationY)
		.Z(this, &FWireNodeDetails::GetLocationZ)
		.AllowResponsiveLayout(true)
		.AllowSpin(false)
		.OnXCommitted(this, &FWireNodeDetails::OnSetLocation, 0)
		.OnYCommitted(this, &FWireNodeDetails::OnSetLocation, 1)
		.OnZCommitted(this, &FWireNodeDetails::OnSetLocation, 2)
	];

	NodeTypeComboBox = SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&WireNodeTypes)
			.OnGenerateWidget(this, &FWireNodeDetails::OnGenerateComboWidget)
			.OnSelectionChanged(this, &FWireNodeDetails::OnNodeTypeChanged)
		[
				SNew(STextBlock)
				.Text(this, &FWireNodeDetails::GetNodeType)
		];

	ChildrenBuilder.AddCustomRow(LOCTEXT("Type", "Type"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithSelection))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Type", "Type"))
	]
	.ValueContent()
	[
		NodeTypeComboBox.ToSharedRef()
	];

	//clang-format on
}

bool FWireNodeDetails::InitiallyCollapsed() const
{
	return false;
}

FName FWireNodeDetails::GetName() const
{
	return TEXT("Wire Node Details");
}

bool FWireNodeDetails::RequiresTick() const
{
	return true;
}

void FWireNodeDetails::Tick(float DeltaTime)
{
	UpdateValues();
}

void FWireNodeDetails::UpdateValues()
{
	Wire = WireVisualizer->GetSelectedWire();
	SelectedNodeIndex = WireVisualizer->GetSelectedNodeIndex();
	if (Wire == nullptr || SelectedNodeIndex == INDEX_NONE)
	{
		LocationX.Reset();
		LocationY.Reset();
		LocationZ.Reset();
		NodeType.Reset();
		return;
	}

	const FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];
	const FVector Location = Node.Location;
	LocationX = Location.X;
	LocationY = Location.Y;
	LocationZ = Location.Z;
	NodeType = Node.NodeType;

	const int32 EnumIndex = static_cast<int32>(NodeType.GetValue());
	if (NodeTypeComboBox->GetSelectedItem() != WireNodeTypes[EnumIndex])
	{
		NodeTypeComboBox->SetSelectedItem(WireNodeTypes[EnumIndex]);
	}
}

EVisibility FWireNodeDetails::WithSelection() const
{
	if (Wire != nullptr && SelectedNodeIndex != INDEX_NONE)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility FWireNodeDetails::WithoutSelection() const
{
	if (Wire == nullptr || SelectedNodeIndex == INDEX_NONE)
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

TOptional<float> FWireNodeDetails::GetLocationX() const
{
	return LocationX;
}

TOptional<float> FWireNodeDetails::GetLocationY() const
{
	return LocationY;
}

TOptional<float> FWireNodeDetails::GetLocationZ() const
{
	return LocationZ;
}

void FWireNodeDetails::OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis)
{
	if (Wire == nullptr || SelectedNodeIndex == INDEX_NONE)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetWireNodeLocation", "Set wire node location"));
	Wire->Modify();

	FVector Location = Wire->RouteNodes[SelectedNodeIndex].Location;
	Location.Component(Axis) = NewValue;
	Wire->RouteNodes[SelectedNodeIndex].Location = Location;

	FComponentVisualizer::NotifyPropertyModified(
		Wire, FindFProperty<FProperty>(
				  UAGX_WireComponent::StaticClass(),
				  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));

	UpdateValues();
}

/// @todo The color stuff is copy/paste from AGX_WireComponentVisualizer.cpp. Figure out how to have
/// only one.
#if 0
TStaticArray<FLinearColor, 3> CreateWireNodeColors()
{
	TStaticArray<FLinearColor, 3> WireNodeColors;
	WireNodeColors[0] = FLinearColor::Red;
	WireNodeColors[1] = FLinearColor::Green;
	WireNodeColors[2] = FLinearColor::Blue;
	return WireNodeColors;
}

FLinearColor WireNodeTypeToColor(EWireNodeType Type)
{
	static TStaticArray<FLinearColor, 3> WireNodeColors = CreateWireNodeColors();
	const uint32 I = static_cast<uint32>(Type);
	return WireNodeColors[I];
}
#else
FLinearColor WireNodeTypeToColor(EWireNodeType Type);
#endif
FLinearColor WireNodeTypeIndexToColor(int32 Type)
{
	return WireNodeTypeToColor(static_cast<EWireNodeType>(Type));
}

TSharedRef<SWidget> FWireNodeDetails::OnGenerateComboWidget(TSharedPtr<FString> InComboString)
{
	const int32 EnumIndex = WireNodeTypes.Find(InComboString);
	const FLinearColor Color = WireNodeTypeIndexToColor(EnumIndex);
	// clang-format off
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InComboString))
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SColorBlock)
			.Color(Color)
			//.ColorAndOpacity(FSlateColor(Color))
		];
	// clang-format on
}

FText FWireNodeDetails::GetNodeType() const
{
	if (NodeType.IsSet())
	{
		const int32 EnumIndex = static_cast<int32>(NodeType.GetValue());
		return FText::FromString(*WireNodeTypes[EnumIndex]);
	}
	else
	{
		return LOCTEXT("NoSelection", "(Nothing Selected)");
	}
}

void FWireNodeDetails::OnNodeTypeChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (Wire == nullptr || SelectedNodeIndex == INDEX_NONE)
	{
		return;
	}

	if (!Wire->RouteNodes.IsValidIndex(SelectedNodeIndex))
	{
		UpdateValues();
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetWireNodeType", "Set wire node type"));
	Wire->Modify();

	const int32 EnumIndex = WireNodeTypes.Find(NewValue);
	Wire->RouteNodes[SelectedNodeIndex].NodeType = static_cast<EWireNodeType>(EnumIndex);
	FComponentVisualizer::NotifyPropertyModified(
		Wire,
		FindFProperty<FProperty>(
			UAGX_WireComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));
	UpdateValues();
}

void FWireNodeDetails::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

TSharedRef<IDetailCustomization> FAGX_WireComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_WireComponentCustomization());
}

void FAGX_WireComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Selected Node");

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(ObjectsBeingCustomized[0]);
	if (Wire == nullptr)
	{
		return;
	}

	TSharedRef<FWireNodeDetails> WireNodeDetails = MakeShareable(new FWireNodeDetails(Wire));
	Category.AddCustomBuilder(WireNodeDetails);
}

#undef LOCTEXT_NAMESPACE
