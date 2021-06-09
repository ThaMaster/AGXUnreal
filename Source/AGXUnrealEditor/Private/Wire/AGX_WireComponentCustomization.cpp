#include "Wire/AGX_WireComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireComponentVisualizer.h"
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "UnrealEd.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_WireComponentCustomization"

/**
 * A Slate builder that produces the widgets exposing the properties for a single selected wire
 * node.
 */
class FWireNodeDetails : public IDetailCustomNodeBuilder, public TSharedFromThis<FWireNodeDetails>
{
	/*
	 * Slate functionality is organized in layers held and orchestrated by the owning class.
	 * - Widgets. The pixels on the screen. Rendered very frame.
	 * - Callbacks. Functions called by the widget.
	 * - Storage. In-class backing storage read by the widget renderers.
	 * - Objects. The actual objects that the widgets and storage represents.
	 *
	 * When the widgets are created function pointers are passed and registered as callbacks. There
	 * are callbacks both for getting the current value to be rendered, called a read callback, and
	 * for performing some action when the end-user interacts with the widget, called a write
	 * callback.
	 *
	 * Crucially, the widget is NOT regenerated/recreated when the state changes so these callbacks
	 * must be able to accommodate any possible change to the underlying objects.
	 *
	 * The storage layer consists of TOptionals, for simple values, and TArray<TSharedPtr>s, for
	 * selection lists, that the widget uses for its function. Sometimes the widget knows about
	 * the storage directly, e.g., SComboBox and its source, and sometimes the read callback returns
	 * a value to be rendered by reading from the storage, e.g., SVectorInputBox.
	 *
	 * It is the responsibility of the Tick virtual member function to synchronize the storage layer
	 * with the current object state. If that is not possible in some cases then call ExecuteIfBound
	 * on the FSimpleDelegate passed to SetOnRebuildChildren during initialization. This will
	 * recreate everything so consider this a last fallback.
	 */
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

private:
	/*
	 * WIDGETS
	 * Only widgets for which we need functionality beyond its supported callbacks are stored.
	 */

	/// Used to select between Free, Eye, and BodyFixed.
	/// Stored because we need to change the selected item when switching between nodes.
	TSharedPtr<SComboBox<TSharedPtr<FString>>> NodeTypeComboBox;

	/// Used to select which body among those in an Actor that an Eye or BodyFixed should attach to.
	/// Stored because it must be rebuilt when a new node is selected or the Rigid Body is changed.
	TSharedPtr<SComboBox<TSharedPtr<FString>>> BodyNameComboBox;

	/*
	 * CALLBACKS
	 * I use OnSet for all write callbacks and OnGet for all read callbacks.
	 */

	/* Selection. */

	// Getters.

	FText OnGetSelectedNodeIndexText() const;
	/* Location. */

	// Getters.

	/** Callbacks called when the Location widget is rendered. */
	TOptional<float> OnGetLocationX() const;
	TOptional<float> OnGetLocationY() const;
	TOptional<float> OnGetLocationZ() const;

	// Setters.

	/// Callback called when the Location widget is edited.
	void OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis);

	/* Node type. */

	// Getters.

	/// Called for each entry in the WireNodeTypes combo box, to generate the list entries.
	TSharedRef<SWidget> OnGetNodeTypeEntryWidget(TSharedPtr<FString> InComboString);

	/// Called to generate the default (non-opened) view of the node type combo box.
	FText OnGetNodeTypeLabel() const;

	// Setters.

	/// Called when the end-user selects an entry in the node type combo box.
	void OnSetNodeType(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	/* Rigid body. */

	// Getters.

	/// Called to generate the default (non-opened) view of the rigid body combo box.
	FText OnGetRigidBodyLabel() const;

	/// Called to get the color of the Rigid Body name. Used to highlight invalid selections.
	FSlateColor OnGetRigidBodyNameColor() const;

	/// Called to generated the label identifying the currently selected body owning Actor.
	FText OnGetRigidBodyOwnerLabel() const;

	/// Called to test if the Actor can be used as the body owner, i.e., if it has a rigid body.
	bool OnGetHasRigidBody(const AActor* Actor);

	/// Called to limit the set of types that can be picked when selecting rigid body owner.
	void OnGetAllowedClasses(TArray<const UClass*>& AllowedClasses);

	/// Called to limit the set of Actors that can be selected as the rigid body owner.
	void OnGetActorFilters(TSharedPtr<SceneOutliner::FOutlinerFilters>& Filters);

	/// Called for each entry in the RigidBody combo box, to generate the list entries.
	TSharedRef<SWidget> OnGetRigidBodyEntryWidget(TSharedPtr<FString> InComboString);

	// Setters.

	/// Called when the end-user picks a rigid body owner Actor.
	void OnSetRigidBodyOwner(AActor* Actor);

	/// Called when the end-user selects an entry in the rigid body combo box.
	void OnSetRigidBody(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	/*
	 * STORAGE
	 */

	// Backing storage for the selected node's location.
	TOptional<float> LocationX;
	TOptional<float> LocationY;
	TOptional<float> LocationZ;

	// Backing storage for the selected node's type.
	TOptional<EWireNodeType> NodeType;

	// Backing storage for the node type combo box.
	TArray<TSharedPtr<FString>> WireNodeTypes;

	/// @todo Consider making this a FName array instead.
	// Backing storage for the rigid body name combo box.
	TArray<TSharedPtr<FString>> RigidBodyNames;

	/*
	 * OBJECTS
	 */

	/**
	 * The currently active Wire Component Visualizer. Not sure how this works with multiple Detail
	 * Panels, selections, and Viewports.
	 */
	FAGX_WireComponentVisualizer* WireVisualizer;

	/// The currently selected Wire Component.
	UAGX_WireComponent* Wire;

	/**
	 * The index of the currently selected node in the Wire Component Visualizer. This is the node
	 * whose properties is shown in the widgets, and edits made in the widgets will be applied to
	 * this node. Can be INDEX_NONE.
	 */
	int32 SelectedNodeIndex;

private:
	/**
	 * @return true if we have a wire and a node index that is valid for that wire.
	 */
	bool HasWireAndNodeSelection() const;

	/**
	 * Update the Details Panel state from the selected wire and node.
	 *
	 * Resets the wire and node selection if incompatible edits has been made elsewhere.
	 *
	 * Copy node properties from the selected node, if any, into the backing storage.
	 */
	void UpdateValues();

	/// Widgets with this visibility is only shown when a wire node is selected.
	EVisibility WithSelection() const;

	/// Widgets with this visibility is only shown when no wire node is selected.
	EVisibility WithoutSelection() const;

	/// Widgets with this visibility is only shown when the selected wire node type needs a body.
	EVisibility NodeHasRigidBody() const;

private:
	/// Delegate that the Slate system provides for us. When executed a rebuild is triggered.
	FSimpleDelegate OnRegenerateChildren;

	/// Some of the callbacks manipulate other widgets. In these cases the original callback is the
	/// one responsible for maintaining all invariants and the transitive callbacks should not do
	/// anything since that will interfere with the original callback. We signal that we already
	/// have a callback on the stack by setting this flag, which is tested at the top of each
	/// callback.
	bool bIsRunningCallback = false;
};

FWireNodeDetails::FWireNodeDetails(UAGX_WireComponent* InWire)
{
	// Get the Wire Component Visualizer for this type of wire.
	check(InWire);
	FComponentVisualizer* Visualizer = GUnrealEd->FindComponentVisualizer(InWire->GetClass()).Get();
	WireVisualizer = (FAGX_WireComponentVisualizer*) Visualizer;
	check(WireVisualizer);

	// Create backing storage for the node type combo box.
	UEnum* NodeTypesEnum = StaticEnum<EWireNodeType>();
	check(NodeTypesEnum);
	for (int32 EnumIndex = 0; EnumIndex < (int32) EWireNodeType::NUM_USER_CREATABLE; ++EnumIndex)
	{
		WireNodeTypes.Add(
			MakeShareable(new FString(NodeTypesEnum->GetNameStringByIndex(EnumIndex))));
	}

	// Should always have the emptry string entry in the body names list, so that it's possible
	// to select None.
	RigidBodyNames.Add(MakeShareable(new FString("")));

	// Build initial state. Clearing Wire and SelectedNodeIndex to ensure that this first selection
	// is seen as a new one.
	Wire = nullptr;
	SelectedNodeIndex = INDEX_NONE;
	UpdateValues();
}

//~ Begin IDetailCustomNodeBuilder interface

void FWireNodeDetails::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
}

void FWireNodeDetails::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	// clang-format off

	// "No Selection" marker widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("NoSelection", "No Selection"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithoutSelection))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("NoNodeSelection", "No node is selected"))
	];

	// "Selected Node" title widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("Title", "Title"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithSelection))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(this, &FWireNodeDetails::OnGetSelectedNodeIndexText)
	];

	// Location widget.
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
		.X(this, &FWireNodeDetails::OnGetLocationX)
		.Y(this, &FWireNodeDetails::OnGetLocationY)
		.Z(this, &FWireNodeDetails::OnGetLocationZ)
		.AllowResponsiveLayout(true)
		.AllowSpin(false)
		.OnXCommitted(this, &FWireNodeDetails::OnSetLocation, 0)
		.OnYCommitted(this, &FWireNodeDetails::OnSetLocation, 1)
		.OnZCommitted(this, &FWireNodeDetails::OnSetLocation, 2)
	];

	// Node type widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("Type", "Type"))
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::WithSelection))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Type", "Type"))
	]
	.ValueContent()
	[
		SAssignNew(NodeTypeComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&WireNodeTypes)
			.OnGenerateWidget(this, &FWireNodeDetails::OnGetNodeTypeEntryWidget)
			.OnSelectionChanged(this, &FWireNodeDetails::OnSetNodeType)
			[
				SNew(STextBlock)
				.Text(this, &FWireNodeDetails::OnGetNodeTypeLabel)
			]
	];

	// Group to hold the two Rigid Body widgets.
	IDetailGroup& RigidBody = ChildrenBuilder.AddGroup(
		TEXT("RigidBodyTitle"), LOCTEXT("RigidBodyTitle", "RigidBody"));

	// Rigid Body owning Actor widget.
	RigidBody.AddWidgetRow()
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::NodeHasRigidBody))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("OwningActor", "Owning Actor"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
				SNew(STextBlock)
				.Text(this, &FWireNodeDetails::OnGetRigidBodyOwnerLabel)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.MaxDesiredWidth(40)
			.MaxDesiredHeight(20)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					// Creates a button that opens a menu with a list of actors in the level.
					PropertyCustomizationHelpers::MakeActorPickerAnchorButton(
						FOnGetActorFilters::CreateSP(this, &FWireNodeDetails::OnGetActorFilters),
						FOnActorSelected::CreateSP(this, &FWireNodeDetails::OnSetRigidBodyOwner))
				]
				+ SHorizontalBox::Slot()
				[
					// Creates a button that enables the Actor Picker mode.
					PropertyCustomizationHelpers::MakeInteractiveActorPicker(
						FOnGetAllowedClasses::CreateSP(this, &FWireNodeDetails::OnGetAllowedClasses),
						FOnShouldFilterActor::CreateSP(this, &FWireNodeDetails::OnGetHasRigidBody),
						FOnActorSelected::CreateSP(this, &FWireNodeDetails::OnSetRigidBodyOwner))
				]
			]
		]
	];

	// Rigid Body name widget.
	RigidBody.AddWidgetRow()
	.Visibility(TAttribute<EVisibility>(this, &FWireNodeDetails::NodeHasRigidBody))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("RigidBodyName", "Rigid Body Name"))
	]
	.ValueContent()
	[
		SAssignNew(BodyNameComboBox, SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&RigidBodyNames)
		.OnGenerateWidget(this, &FWireNodeDetails::OnGetRigidBodyEntryWidget)
		.OnSelectionChanged(this, &FWireNodeDetails::OnSetRigidBody)
		[
			SNew(STextBlock)
			.Text(this, &FWireNodeDetails::OnGetRigidBodyLabel)
			.ColorAndOpacity(this, &FWireNodeDetails::OnGetRigidBodyNameColor)
		]
	];

	// clang-format on
}

bool FWireNodeDetails::InitiallyCollapsed() const
{
	return false;
}

void FWireNodeDetails::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
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

//~ End IDetailCustomNodeBuilder interface

namespace WireNodeDetails_helpers
{
	bool NodeTypeHasBody(EWireNodeType NodeType)
	{
		return NodeType == EWireNodeType::BodyFixed || NodeType == EWireNodeType::Eye;
	}

	/**
	 * Replace the contents of RigidBodyNames with the names of the Rigid Body Components in Actor.
	 * An empty string is added before the names to make it possible to select "nothing".
	 * A nullptr Actor produces a list containing only the empty string.
	 *
	 * @param RigidBodyNames The array to fill with names. Original contents is discarded.
	 * @param Actor The Actor to search for Rigid Body Components in.
	 */
	void RebuildRigidBodyNames(TArray<TSharedPtr<FString>>& RigidBodyNames, AActor* Actor)
	{
		if (Actor == nullptr)
		{
			RigidBodyNames.Reset(1);
			RigidBodyNames.Add(MakeShareable(new FString("")));
			return;
		}

		TArray<UAGX_RigidBodyComponent*> RigidBodyComponents;
		Actor->GetComponents(RigidBodyComponents);
		RigidBodyNames.Reset(RigidBodyComponents.Num() + 1);
		RigidBodyNames.Add(MakeShareable(new FString("")));
		for (const auto& Body : RigidBodyComponents)
		{
			RigidBodyNames.Add(MakeShareable(new FString(Body->GetName())));
		}
	}

	int32 FindRigidBodyName(
		const FString& RigidBodyName, const TArray<TSharedPtr<FString>>& RigidBodyNames)
	{
		const TSharedPtr<FString>* Element =
			RigidBodyNames.FindByPredicate([RigidBodyName](const TSharedPtr<FString>& Element) {
				return *Element == RigidBodyName;
			});
		if (Element == nullptr)
		{
			return INDEX_NONE;
		}

		return Element - &RigidBodyNames[0];
	}

	/**
	 * Rebuild the Rigid Body combo box.
	 *
	 * Selects the ToSelect entry if it exists. Selects the empty string entry if ToSelect doesn't
	 * exist in the updated list.
	 *
	 * Call this when the Details Panel is being updated without changing the Node.
	 * @see RebuildRigidBodyComboBox_Edit
	 *
	 *
	 * @param BodyNameComboBox The Combo Box that displays the names.
	 * @param RigidBodyNames List of names that is to be rebuilt.
	 * @param ToSelect The contents of the entry which we wish to select in the Combo Box.
	 * @param Actor The Actor in which we search for Rigid Body Components.
	 */
	void RebuildRigidBodyComboBox_View(
		SComboBox<TSharedPtr<FString>>& BodyNameComboBox,
		TArray<TSharedPtr<FString>>& RigidBodyNames, const FString& ToSelect, AActor* Actor)
	{
		BodyNameComboBox.ClearSelection();
		RebuildRigidBodyNames(RigidBodyNames, Actor);
		BodyNameComboBox.RefreshOptions();

		int32 Index = FindRigidBodyName(ToSelect, RigidBodyNames);
		if (Index == INDEX_NONE)
		{
			// The rigid body name we had selected doesn't exist anymore. We're in a context where
			// we can't edit the underlying node so select the empty string entry to ensure that
			// the actual body name entries are selectable.
			Index = 0;
		}
		BodyNameComboBox.SetSelectedItem(RigidBodyNames[Index]);
	}

	/**
	 * Rebuilt the Rigid Body combo box.
	 *
	 * Selects the ToSelect entry if it exists. Selects the first non-empty entry if ToSelect
	 * doesn't exist. Selects the empty string entry if there is no non-empty entry.
	 *
	 * Call this when the Details Panel is being updated in response to am edit.
	 * @see RebuildRigidBodyComboBox_View
	 *
	 * The returned string should be assigned to the node's Rigid Body name.
	 *
	 * @param BodyNameComboBox The Combo Box that displays the names.
	 * @param RigidBodyNames List of names that is to be rebuilt.
	 * @param ToSelect The contents of the entry which we wish to select in the Combo Box.
	 * @param Actor The Actor in which we search for Rigid Body Components.
	 */
	FString RebuildRigidBodyComboBox_Edit(
		SComboBox<TSharedPtr<FString>>& BodyNameComboBox,
		TArray<TSharedPtr<FString>>& RigidBodyNames, const FString& ToSelect, AActor* Actor)
	{
		BodyNameComboBox.ClearSelection();
		RebuildRigidBodyNames(RigidBodyNames, Actor);
		BodyNameComboBox.RefreshOptions();

		int32 Index = FindRigidBodyName(ToSelect, RigidBodyNames);
		if (Index == INDEX_NONE)
		{
			// The rigid body name we had selected doesn't exist anymore. Select either the empty
			// string, at index 0, or the first rigid body name, at index 1.
			// RebuildRigidBodyNames guarantees that Num() - 1 will never be negative.
			Index = FMath::Min(RigidBodyNames.Num() - 1, 1);
		}
		BodyNameComboBox.SetSelectedItem(RigidBodyNames[Index]);
		return *RigidBodyNames[Index];
	}
}

// Begin selection getters.

FText FWireNodeDetails::OnGetSelectedNodeIndexText() const
{
	return FText::Format(
		LOCTEXT("NodeIndexText", "Properties of the currently selected node. {0}"),
		FText::AsNumber(SelectedNodeIndex));
}

// Begin Location getters.

TOptional<float> FWireNodeDetails::OnGetLocationX() const
{
	return LocationX;
}

TOptional<float> FWireNodeDetails::OnGetLocationY() const
{
	return LocationY;
}

TOptional<float> FWireNodeDetails::OnGetLocationZ() const
{
	return LocationZ;
}

// Begin location setters.

void FWireNodeDetails::OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis)
{
	if (!HasWireAndNodeSelection())
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
}

// Begin node type getters.

/// @todo Hacky import from AGX_WireComponentVisualizer.cpp. Decide where to put this function.
namespace AGX_WireComponentVisualizer_helpers
{
	FLinearColor WireNodeTypeToColor(EWireNodeType Type);
}

FLinearColor WireNodeTypeIndexToColor(int32 Type)
{
	using namespace AGX_WireComponentVisualizer_helpers;
	return WireNodeTypeToColor(static_cast<EWireNodeType>(Type));
}

TSharedRef<SWidget> FWireNodeDetails::OnGetNodeTypeEntryWidget(TSharedPtr<FString> InComboString)
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
			/// @todo Put a SBox here to reduce the width of the Color Block. Try to make it square.
			SNew(SColorBlock)
			.Color(Color)
		];
	// clang-format on
}

FText FWireNodeDetails::OnGetNodeTypeLabel() const
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

// Begin node type setters.

void FWireNodeDetails::OnSetNodeType(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	using namespace WireNodeDetails_helpers;

	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	if (!HasWireAndNodeSelection())
	{
		return;
	}

	FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];

	const int32 EnumIndex = WireNodeTypes.Find(NewValue);
	const EWireNodeType NewNodeType = static_cast<EWireNodeType>(EnumIndex);

	const bool OldHadBody = NodeTypeHasBody(Node.NodeType);
	const bool NewHasBody = NodeTypeHasBody(NewNodeType);
	FString NewBodyName;
	if (!OldHadBody && NewHasBody)
	{
		// The RigidBody selector is about to be shown. Prepare it's backing storage.
		NewBodyName = RebuildRigidBodyComboBox_Edit(
			*BodyNameComboBox, RigidBodyNames, Node.RigidBody.BodyName.ToString(),
			Node.RigidBody.OwningActor);
	}

	const FScopedTransaction Transaction(LOCTEXT("SetWireNodeType", "Set wire node type"));
	Wire->Modify();

	Node.NodeType = NewNodeType;
	Node.RigidBody.BodyName = FName(*NewBodyName);

	FComponentVisualizer::NotifyPropertyModified(
		Wire, FindFProperty<FProperty>(
				  UAGX_WireComponent::StaticClass(),
				  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));
}

// Begin rigid body getters.

FText FWireNodeDetails::OnGetRigidBodyLabel() const
{
	if (!HasWireAndNodeSelection())
	{
		return LOCTEXT("NoSelection", "(Nothing Selected)");
	}
	FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];
	return FText::FromName(Node.RigidBody.BodyName);
}

FSlateColor FWireNodeDetails::OnGetRigidBodyNameColor() const
{
	// Here I would like to get the default text color so that the Rigid Body name looks just like
	// it would if we hadn't had this callback in most cases. But I don't know how to do that, so
	// defaulting to black for now.
	const FLinearColor DefaultColor = FLinearColor::Black;
	// The following are some of my attempts.
	// FLinearColor DefaultColor = FSlateColor::UseForeground().GetColor(FWidgetStyle());
	// FLinearColor DefaultColor = FCoreStyle::GetDefaultFont().Get()->GetColor(FWidgetStyle());

	if (BodyNameComboBox->GetSelectedItem().IsValid() &&
		BodyNameComboBox->GetSelectedItem() == RigidBodyNames[0])
	{
		// The first element of RigidBodyNames is the "unknown" marker. It is selected when the
		// underlying node doesn't have a Rigid Body name, and when the name it does have doen't
		// correspond to any Rigid Body in the Rigid Body owning Actor.
		return FSlateColor(FLinearColor::Red);
	}

	return FSlateColor(DefaultColor);
}

FText FWireNodeDetails::OnGetRigidBodyOwnerLabel() const
{
	if (!HasWireAndNodeSelection())
	{
		return LOCTEXT("NoSelection", "(Nothing Selected)");
	}
	FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];
	const FString Label = GetLabelSafe(Node.RigidBody.OwningActor);
	return FText::FromString(Label);
}

bool FWireNodeDetails::OnGetHasRigidBody(const AActor* Actor)
{
	const UActorComponent* const* It = Actor->GetInstanceComponents().FindByPredicate(
		[](const UActorComponent* const C) { return C->IsA<UAGX_RigidBodyComponent>(); });

	return It != nullptr;
}

void FWireNodeDetails::OnGetAllowedClasses(TArray<const UClass*>& AllowedClasses)
{
	AllowedClasses.Add(AActor::StaticClass());
}

void FWireNodeDetails::OnGetActorFilters(TSharedPtr<SceneOutliner::FOutlinerFilters>& Filters)
{
	/// @todo What should we do here?
}

TSharedRef<SWidget> FWireNodeDetails::OnGetRigidBodyEntryWidget(TSharedPtr<FString> InComboString)
{
	return SNew(STextBlock).Text(FText::FromString(*InComboString));
}

// Begin rigid body setters.

void FWireNodeDetails::OnSetRigidBody(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	if (!HasWireAndNodeSelection())
	{
		return;
	}

	FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];

	FName NewName = NewValue.IsValid() ? FName(*NewValue) : NAME_None;
	if (NewName == Node.RigidBody.BodyName)
	{
		return;
	}

	const FScopedTransaction Transaction(
		LOCTEXT("SetWireNodeRigidBodyName", "Set Wire Node Rigid Body Name"));
	Wire->Modify();

	if (NewValue.IsValid())
	{
		Node.RigidBody.BodyName = FName(*NewValue);
	}
	else
	{
		Node.RigidBody.BodyName = NAME_None;
	}
}

void FWireNodeDetails::OnSetRigidBodyOwner(AActor* Actor)
{
	using namespace WireNodeDetails_helpers;

	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	if (!HasWireAndNodeSelection())
	{
		return;
	}

	FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];

	FName NewBodyName = FName(*RebuildRigidBodyComboBox_Edit(
		*BodyNameComboBox, RigidBodyNames, Node.RigidBody.BodyName.ToString(), Actor));

	if (Actor == Node.RigidBody.OwningActor && NewBodyName == Node.RigidBody.BodyName)
	{
		return;
	}

	const FScopedTransaction Transaction(
		LOCTEXT("SetWireNodeBodyActor", "Set wire node body actor"));

	Wire->Modify();
	Node.RigidBody.OwningActor = Actor;
	Node.RigidBody.BodyName = NewBodyName;

	FComponentVisualizer::NotifyPropertyModified(
		Wire, FindFProperty<FProperty>(
				  UAGX_WireComponent::StaticClass(),
				  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));
}

bool FWireNodeDetails::HasWireAndNodeSelection() const
{
	return Wire != nullptr && Wire->RouteNodes.IsValidIndex(SelectedNodeIndex);
}

void FWireNodeDetails::UpdateValues()
{
	using namespace WireNodeDetails_helpers;

	// Sometimes Tick is called before GenerateChildContent, meaning that Unreal Editor is trying
	// to update an empty FWireNodeDetails. We detect this by checking for the presence of the
	// node type Combo Box.
	if (!NodeTypeComboBox.IsValid())
	{
		return;
	}

	const bool bSelectionChanged = WireVisualizer->GetSelectedWire() != Wire ||
								   WireVisualizer->GetSelectedNodeIndex() != SelectedNodeIndex;

	// Get selections from the Wire Component Visualizer.
	Wire = WireVisualizer->GetSelectedWire();
	SelectedNodeIndex = WireVisualizer->GetSelectedNodeIndex();

	// Reset and bail if we don't have a valid selection.
	if (!HasWireAndNodeSelection())
	{
		// Clear all cached state related to the selection.
		LocationX.Reset();
		LocationY.Reset();
		LocationZ.Reset();
		NodeType.Reset();
		RebuildRigidBodyComboBox_View(*BodyNameComboBox, RigidBodyNames, TEXT(""), nullptr);
		Wire = nullptr;
		SelectedNodeIndex = INDEX_NONE;
		return;
	}

	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	const FWireRoutingNode& Node = Wire->RouteNodes[SelectedNodeIndex];

	// Read node location.
	const FVector Location = Node.Location;
	LocationX = Location.X;
	LocationY = Location.Y;
	LocationZ = Location.Z;

	// Read node type.
	NodeType = Node.NodeType;
	const int32 EnumIndex = static_cast<int32>(NodeType.GetValue());
	if (!NodeTypeComboBox->GetSelectedItem().IsValid() ||
		NodeTypeComboBox->GetSelectedItem() != WireNodeTypes[EnumIndex])
	{
		NodeTypeComboBox->SetSelectedItem(WireNodeTypes[EnumIndex]);
	}

	if (bSelectionChanged)
	{
		RebuildRigidBodyComboBox_View(
			*BodyNameComboBox, RigidBodyNames, Node.RigidBody.BodyName.ToString(),
			Node.RigidBody.OwningActor);
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

EVisibility FWireNodeDetails::NodeHasRigidBody() const
{
	using namespace WireNodeDetails_helpers;

	if (Wire == nullptr || SelectedNodeIndex == INDEX_NONE)
	{
		return EVisibility::Collapsed;
	}

	if (!Wire->RouteNodes.IsValidIndex(SelectedNodeIndex))
	{
		return EVisibility::Collapsed;
	}

	if (!NodeType.IsSet())
	{
		return EVisibility::Collapsed;
	}

	if (NodeTypeHasBody(NodeType.GetValue()))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
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
