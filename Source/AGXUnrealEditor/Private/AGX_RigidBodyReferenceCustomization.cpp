#include "AGX_RigidBodyReferenceCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_RigidBodyReference.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Utilities/AGX_PropertyUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Actor.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_RigidBodyReferenceCustomization"

TSharedRef<IPropertyTypeCustomization> FAGX_RigidBodyReferenceCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_RigidBodyReferenceCustomization());
}

void FAGX_RigidBodyReferenceCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> BodyReferenceHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	RefreshStoreReferences(BodyReferenceHandle.Get());

	HeaderRow.NameContent()[BodyReferenceHandle->CreatePropertyNameWidget()];

	if (RigidBodyReference == nullptr)
	{
		// Fall back to default value widget when unable to get a single RigidBodyReference.
		HeaderRow.ValueContent()[BodyReferenceHandle->CreatePropertyValueWidget()];
		return;
	}

	/// \todo Is there a better way to make the text field a bit wider? I want to fill the available
	/// space.
	HeaderRow
		.ValueContent() //
		.MinDesiredWidth(250.0f) // 250 from SPropertyEditorAsset::GetDesiredWidth.
			[SNew(STextBlock)
				 .Text(this, &FAGX_RigidBodyReferenceCustomization::GetHeaderText)
				 .ToolTipText(this, &FAGX_RigidBodyReferenceCustomization::GetHeaderText)
				 .Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				 .MinDesiredWidth(250.0f)]; // 250 from SPropertyEditorAsset::GetDesiredWidth.
}

/**
 * Fetch all names of RigidBodyComponents held by the pointed-to OwningActor. Optionally searches
 * child actors if enabled in the RigidBodyReference instance. The found names are stored internally
 * in this Customization. This member function should be called whenever the owning Actor has been
 * changed, often via RebuildComboBox.
 */
void FetchBodyNamesFromOwner(
	TArray<TSharedPtr<FName>>& BodyNames, AActor* OwningActor,
	TSharedPtr<IPropertyHandle>& SearchChildActorsHandle)
{
	BodyNames.Empty();

	bool bSearchChildActors;
	FPropertyAccess::Result Result = SearchChildActorsHandle->GetValue(bSearchChildActors);
	if (Result != FPropertyAccess::Success)
	{
		// Do not produce a list of body names if multiple RigidBodyReferences are selected. Here we
		// could do something clever to let the user select the same rigid body to many references,
		// but that's too complicated for the first implementation.
		return;
	}

	TArray<UAGX_RigidBodyComponent*> RigidBodyComponents;
	OwningActor->GetComponents(RigidBodyComponents, bSearchChildActors);
	for (UAGX_RigidBodyComponent* RigidBody : RigidBodyComponents)
	{
		BodyNames.Add(MakeShareable(new FName(RigidBody->GetFName())));
	}
}

void FetchBodyNamesFromBlueprint(
	TArray<TSharedPtr<FName>>& BodyNames, IPropertyHandle& BodyReferenceHandle)
{
	/*
	 The purpose of this function is to collect the names of all RigidBodyComponents in the
	 currently open Blueprint so that they can be used to populate a ComboBox in e.g. the constraint
	 details panel. For some reason the number of components is always zero. Not even the Component
	 currently being customized shows up and it is be definition included in the Blueprint.
	 Something is wrong.
	 */

	BodyNames.Empty();

	UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(
		FAGX_PropertyUtilities::GetParentObjectOfStruct(BodyReferenceHandle));
	if (Constraint == nullptr)
	{
		return;
	}

	UBlueprintGeneratedClass* Blueprint = Cast<UBlueprintGeneratedClass>(Constraint->GetOuter());
	if (Blueprint == nullptr)
	{
		return;
	}

	UE_LOG(
		LogAGX, Warning, TEXT("Reading body names from Blueprint named '%s'. Have %d components."),
		*Blueprint->GetName(), Blueprint->ComponentTemplates.Num());

	for (UActorComponent* Component : Blueprint->ComponentTemplates)
	{
		UE_LOG(LogAGX, Warning, TEXT("  Checking component '%s'."), *Component->GetName());
		if (UAGX_RigidBodyComponent* RigidBody = Cast<UAGX_RigidBodyComponent>(Component))
		{
			UE_LOG(LogAGX, Warning, TEXT("  Is a body, adding."));
			BodyNames.Add(MakeShareable(new FName(RigidBody->GetFName())));
		}
	}
}

void FAGX_RigidBodyReferenceCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> BodyReferenceHandle, IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	RefreshStoreReferences(BodyReferenceHandle.Get());

	if (RigidBodyReference == nullptr || !OwningActorHandle.IsValid() ||
		!BodyNameHandle.IsValid() || !SearchChildActorsHandle.IsValid())
	{
		return;
	}

	RebuildComboBoxDelegate.BindRaw(this, &FAGX_RigidBodyReferenceCustomization::RebuildComboBox);
	OwningActorHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);
	SearchChildActorsHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);

	SelectedBody = RigidBodyReference->BodyName;
	AActor* OwningActor = GetOwningActor();
	if (OwningActor != nullptr)
	{
		FetchBodyNamesFromOwner(BodyNames, OwningActor, SearchChildActorsHandle);
	}
	else
	{
		// @todo FetchBodyNamesFromBlueprint currently always fails. Find some way to fix this. For
		// now the call is commented out since it floods the log with warning messages.
		// FetchBodyNamesFromBlueprint(BodyNames, BodyReferenceHandle.Get());
	}

	StructBuilder.AddProperty(OwningActorHandle.ToSharedRef());
	StructBuilder.AddProperty(SearchChildActorsHandle.ToSharedRef());

	/// \todo Use CreatePropertyNameWidget here, instead of hard coded string?
	FDetailWidgetRow& NameRow =
		StructBuilder.AddCustomRow(FText::FromString("Rigid Body Component"));
	NameRow.NameContent()[SNew(STextBlock)
							  .Text(FText::FromString("RigidBodyComponent"))
							  .Font(IPropertyTypeCustomizationUtils::GetRegularFont())];

	TSharedRef<SComboBox<TSharedPtr<FName>>> ComboBox =
		SNew(SComboBox<TSharedPtr<FName>>)
			.Visibility_Lambda([this]() {
				return BodyNames.Num() == 0 ? EVisibility::Hidden : EVisibility::Visible;
			})
			.OptionsSource(&BodyNames)
			.OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) {
				return SNew(STextBlock).Text(FText::FromName(*Item));
			})
			.OnSelectionChanged(this, &FAGX_RigidBodyReferenceCustomization::OnComboBoxChanged)
			.Content()[SNew(STextBlock).Text_Lambda([this]() {
				return FText::FromName(SelectedBody);
			})];
	for (TSharedPtr<FName>& BodyName : BodyNames)
	{
		if (*BodyName == SelectedBody)
		{
			ComboBox->SetSelectedItem(BodyName);
		}
		/// \todo What should we do if the current name in the underlying data store doesn't have a
		/// corresponding RigidBody with the same name? Should an entry for that name be added? That
		/// would be deceptive since it would be an illegal selection for the current Actor. Should
		/// it be added but marked somehow? Should we invalidate the underlying data store
		/// immediately? I'm not confortable with GUI validation altering the data store. Feels odd.
	}

	TSharedRef<SEditableTextBox> NameBox =
		SNew(SEditableTextBox)
			.Text_Lambda([this]() { return FText::FromName(SelectedBody); })
			.OnTextCommitted(this, &FAGX_RigidBodyReferenceCustomization::OnBodyNameCommited)
			.Visibility_Lambda([this]() {
				return BodyNames.Num() == 0 ? EVisibility::Visible : EVisibility::Hidden;
			});

	NameRow.ValueContent()
		[SNew(SVerticalBox) + SVerticalBox::Slot()[ComboBox] + SVerticalBox::Slot()[NameBox]];

	ComboBoxPtr = &ComboBox.Get();
	ComponentNameBoxPtr = &NameBox.Get();
}

void FAGX_RigidBodyReferenceCustomization::OnBodyNameCommited(
	const FText& NewName, ETextCommit::Type InCommitType)
{
	SelectedBody = FName(*NewName.ToString());
	if (RigidBodyReference != nullptr)
	{
		/// \todo A write to RigidBodyReference->BodyName is not the same as
		/// BodyNameHandle->SetValue. Not sure why, but the RigidBodyReference seen by
		/// AGX_ConstraintComponentVisualizer is not the same as the RigidBodyReference that is
		/// being customized here. Then how should we handle cache invalidation?
		/// Is there a way to get the actual RigidBodyReference from the BodyNameHandle?
		/// Should we invalidate all caches on BeginPlay?
		BodyNameHandle->SetValue(SelectedBody);
		RigidBodyReference->InvalidateCache();
	}
}

void FAGX_RigidBodyReferenceCustomization::RefreshStoreReferences(
	IPropertyHandle& BodyReferenceHandle)
{
	ComboBoxPtr = nullptr;
	RigidBodyReference = nullptr;
	OwningActorHandle = nullptr;
	BodyNameHandle = nullptr;
	SearchChildActorsHandle = nullptr;

	RigidBodyReference = [&BodyReferenceHandle]() -> FAGX_RigidBodyReference* {
		void* UntypedPointer = nullptr;
		FPropertyAccess::Result Result = BodyReferenceHandle.GetValueData(UntypedPointer);
		if (Result != FPropertyAccess::Success)
		{
			return nullptr;
		}
		return static_cast<FAGX_RigidBodyReference*>(UntypedPointer);
	}();
	if (RigidBodyReference == nullptr)
	{
		return;
	}

	OwningActorHandle = BodyReferenceHandle.GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_RigidBodyReference, OwningActor));
	BodyNameHandle = BodyReferenceHandle.GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_RigidBodyReference, BodyName));
	SearchChildActorsHandle = BodyReferenceHandle.GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_RigidBodyReference, bSearchChildActors));
}

FText FAGX_RigidBodyReferenceCustomization::GetHeaderText() const
{
	AActor* OwningActor = RigidBodyReference->GetOwningActor();
	FName BodyName = RigidBodyReference->BodyName;
	FName ActorName = OwningActor ? OwningActor->GetFName() : NAME_None;
	FString Header = FString::Printf(TEXT("%s in %s"), *BodyName.ToString(), *ActorName.ToString());
	return FText::FromString(Header);
}

void FAGX_RigidBodyReferenceCustomization::RebuildComboBox()
{
	SelectedBody = NAME_None;

	AActor* OwningActor = GetOwningActor();
	if (OwningActor != nullptr)
	{
		FetchBodyNamesFromOwner(BodyNames, GetOwningActor(), SearchChildActorsHandle);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_RigidBodyReferenceCustomization::RebuildComboBox called with nullptr owner. "
				 "That's not supposed to happen."));
		BodyNames.Empty();
		return;
	}

	if (ComboBoxPtr == nullptr)
	{
		return;
	}
	ComboBoxPtr->RefreshOptions();
	if (BodyNames.Num() == 0)
	{
		return;
	}

	// Will invalidate the RigidBodyReference cache.
	ComboBoxPtr->SetSelectedItem(BodyNames[0]);
}

void FAGX_RigidBodyReferenceCustomization::OnComboBoxChanged(
	TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectionInfo)
{
	SelectedBody = NewSelection.IsValid() ? *NewSelection : NAME_None;
	RigidBodyReference->BodyName = SelectedBody;
	RigidBodyReference->InvalidateCache();
}

AActor* FAGX_RigidBodyReferenceCustomization::GetOwningActor()
{
	return RigidBodyReference->GetOwningActor();
}

#undef LOCTEXT_NAMESPACE
