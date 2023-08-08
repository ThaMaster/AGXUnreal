// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ComponentReferenceCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Utilities/AGX_PropertyUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_ComponentReferenceCustomization"

TSharedRef<IPropertyTypeCustomization> FAGX_ComponentReferenceCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ComponentReferenceCustomization());
}

namespace AGX_ComponentReferenceCustomization_helpers
{
	void GetComponentNamesFromOwner(
		TArray<TSharedPtr<FName>>& OutNames, const FAGX_ComponentReference& ComponentReference)
	{
		TArray<UActorComponent*> CompatibleComponents;
		ComponentReference.GetCompatibleComponents(CompatibleComponents);
		OutNames.Empty();
		for (const UActorComponent* Component : CompatibleComponents)
		{
			OutNames.Add(MakeShareable(new FName(Component->GetFName())));
		}
	}

	void GetComponentNamesFromBlueprint(
		TArray<TSharedPtr<FName>>& OutNames, IPropertyHandle& ComponentReferenceHandle,
		TSubclassOf<UActorComponent> Type)
	{
		OutNames.Empty();
		const UActorComponent* OwningComponent = Cast<UActorComponent>(
			FAGX_PropertyUtilities::GetParentObjectOfStruct(ComponentReferenceHandle));
		if (OwningComponent == nullptr)
		{
			return;
		}

		const UBlueprintGeneratedClass* Blueprint =
			Cast<UBlueprintGeneratedClass>(OwningComponent->GetOuter());
		if (Blueprint == nullptr)
		{
			return;
		}

		TArray<UBlueprint*> BlueprintChain;
		UBlueprint::GetBlueprintHierarchyFromClass(Blueprint, BlueprintChain);
		for (const UBlueprint* BP : BlueprintChain)
		{
			for (const USCS_Node* Node : BP->SimpleConstructionScript->GetAllNodes())
			{
				if (Node->ComponentTemplate->IsA(Type))
				{
					const FString Name = [Node, BP]()
					{
						FString Name = Node->ComponentTemplate->GetName();
						Name.RemoveFromEnd(
							BP->SimpleConstructionScript->ComponentTemplateNameSuffix);
						return Name;
					}();

					OutNames.Add(MakeShareable(new FName(*Name)));
				}
			}
		}
	}
}

void FAGX_ComponentReferenceCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> InComponentReferenceHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	RefreshStoreReferences(InComponentReferenceHandle);

	// clang-format off
	HeaderRow
	.NameContent()
	[
		ComponentReferenceHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	[
		SNew(STextBlock)
		.Text(this, &FAGX_ComponentReferenceCustomization::GetHeaderText)
		.ToolTipText(this, &FAGX_ComponentReferenceCustomization::GetHeaderText)
		.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		.MinDesiredWidth(250.0f)
	];
	// clang-format on
}

void FAGX_ComponentReferenceCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> InComponentReferenceHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	using namespace AGX_ComponentReferenceCustomization_helpers;

	RefreshStoreReferences(InComponentReferenceHandle);

	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization::CustomizeChildren got nullptr Component "
				 "Reference."));
		return;
	}

	if (!OwningActorHandle.IsValid() || !NameHandle.IsValid() || !SearchChildActorsHandle.IsValid())
	{
		return;
	}

	RebuildComboBoxDelegate.BindRaw(this, &FAGX_ComponentReferenceCustomization::RebuildComboBox);
	OwningActorHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);
	SearchChildActorsHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);

	SelectedComponent = ComponentReference->Name;
	const AActor* OwningActor = GetOwningActor();
	if (OwningActor != nullptr)
	{
		GetComponentNamesFromOwner(ComponentNames, *ComponentReference);
	}
	else
	{
		GetComponentNamesFromBlueprint(
			ComponentNames, *ComponentReferenceHandle.Get(), ComponentReference->ComponentType);
	}

	ComponentNames.Sort([](const TSharedPtr<FName>& Lhs, const TSharedPtr<FName>& Rhs)
						{ return FNameLexicalLess()(*Lhs, *Rhs); });

	ComponentNames.Add(MakeShareable(new FName(TEXT("None"))));

	ChildBuilder.AddProperty(OwningActorHandle.ToSharedRef());
	ChildBuilder.AddProperty(SearchChildActorsHandle.ToSharedRef());

	FDetailWidgetRow& NameRow = ChildBuilder.AddCustomRow(FText::FromString("Component"));
	NameRow.NameContent()[SNew(STextBlock)
							  .Text(FText::FromString("Component"))
							  .Font(IPropertyTypeCustomizationUtils::GetRegularFont())];

	const TSharedRef<SComboBox<TSharedPtr<FName>>> ComboBox =
		SNew(SComboBox<TSharedPtr<FName>>)
			.Visibility_Lambda(
				[this]() {
					return ComponentNames.Num() == 0 ? EVisibility::Collapsed
													 : EVisibility::Visible;
				})
			.OptionsSource(&ComponentNames)
			.OnGenerateWidget_Lambda([](const TSharedPtr<FName>& Item)
									 { return SNew(STextBlock).Text(FText::FromName(*Item)); })
			.OnSelectionChanged(this, &FAGX_ComponentReferenceCustomization::OnComboBoxChanged)
			.Content()[SNew(STextBlock)
						   .Text_Lambda([this]() { return FText::FromName(SelectedComponent); })];

	for (TSharedPtr<FName>& ComponentName : ComponentNames)
	{
		if (*ComponentName == SelectedComponent)
		{
			ComboBox->SetSelectedItem(ComponentName);
		}
	}

	const TSharedRef<SEditableTextBox> NameBox =
		SNew(SEditableTextBox)
			.Text_Lambda([this]() { return FText::FromName(SelectedComponent); })
			.OnTextCommitted(this, &FAGX_ComponentReferenceCustomization::OnComponentNameCommitted)
			.Visibility_Lambda(
				[this]() {
					return ComponentNames.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed;
				});

	NameRow.ValueContent()[SNew(SVerticalBox) + SVerticalBox::Slot()[ComboBox] + SVerticalBox::Slot()[NameBox]];

	ComboBoxPtr = &ComboBox.Get();
	ComponentNameBoxPtr = &NameBox.Get();
}

FText FAGX_ComponentReferenceCustomization::GetHeaderText() const
{
	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		return LOCTEXT(
			"NoComponentReferece",
			"Component Reference Customization does not have a valid Component Reference");
	}

	const AActor* OwningActor = ComponentReference->GetOwningActor();
	const FName ComponentName = ComponentReference->Name;
	const FName ActorName = OwningActor ? OwningActor->GetFName() : NAME_None;
	return FText::Format(
		LOCTEXT("HeaderText", "{0} in {1}"), FText::FromName(ComponentName),
		FText::FromName(ActorName));
}

void FAGX_ComponentReferenceCustomization::RebuildComboBox()
{
	SelectedComponent = NAME_None;

	FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_ComponentReferenceCustomization::RebuildComboBox foudn a nullptr Component "
				 "Reference. Cannot build Combo Box."));
		return;
	}
	TArray<UActorComponent*> CompatibleComponents;
	ComponentReference->GetCompatibleComponents(CompatibleComponents);
	ComponentNames.Empty();
	for (const UActorComponent* Component : CompatibleComponents)
	{
		ComponentNames.Add(MakeShareable(new FName(Component->GetFName())));
	}

	if (ComboBoxPtr == nullptr)
	{
		return;
	}

	ComboBoxPtr->RefreshOptions();
	if (ComponentNames.Num() == 0)
	{
		return;
	}

	ComboBoxPtr->SetSelectedItem(ComponentNames[0]);
}

void FAGX_ComponentReferenceCustomization::OnComboBoxChanged(
	TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectionInfo)
{
	// @todo Code here.
}

void FAGX_ComponentReferenceCustomization::OnComboBoxCommitted(
	const FText& Text, ETextCommit::Type CommitType)
{
	// @todo Code here.
}

void FAGX_ComponentReferenceCustomization::OnComponentNameCommitted(
	const FText& InText, ETextCommit::Type InCommitType)
{
	SelectedComponent = FName(*InText.ToString());
	if (NameHandle.IsValid() && NameHandle->IsValidHandle())
	{
		NameHandle->SetValue(SelectedComponent);
	}

	FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference != nullptr)
	{
		// If/when we do caching in the Component Reference, then the cache needs to be invalidated
		// here.
	}
}

void FAGX_ComponentReferenceCustomization::RefreshStoreReferences(
	const TSharedRef<IPropertyHandle>& InComponentReferenceHandle)
{
	ComponentReferenceHandle = InComponentReferenceHandle;
	OwningActorHandle = ComponentReferenceHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, OwningActor));
	NameHandle = ComponentReferenceHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, Name));
	SearchChildActorsHandle = ComponentReferenceHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, bSearchChildActors));
}

FAGX_ComponentReference* FAGX_ComponentReferenceCustomization::GetComponentReference() const
{
	if (!ComponentReferenceHandle.IsValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The ComponentReferenceHandle is not valid."));
		return nullptr;
	}
	if (!ComponentReferenceHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The ComponentReferenceHandle is nnot a valid "
				 "handle."));
		return nullptr;
	}

	void* UntypedPointer = nullptr;
	const FPropertyAccess::Result Result = ComponentReferenceHandle->GetValueData(UntypedPointer);
	if (Result != FPropertyAccess::Success)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Failed to read value data form Property "
				 "Handle."));
		return nullptr;
	}
	if (UntypedPointer == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Got null Component Reference from the handle."));
		return nullptr;
	}

	return static_cast<FAGX_ComponentReference*>(UntypedPointer);
}

AActor* FAGX_ComponentReferenceCustomization::GetOwningActor() const
{
	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_ComponentReferenceCustomization: GetOwningActor got nullptr Component "
				 "Reference."));
		return nullptr;
	}

	return ComponentReference->GetOwningActor();
}

#undef LOCTEXT_NAMESPACE
