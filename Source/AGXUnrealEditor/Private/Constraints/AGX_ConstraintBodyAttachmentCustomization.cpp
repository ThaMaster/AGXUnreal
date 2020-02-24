#include "Constraints/AGX_ConstraintBodyAttachmentCustomization.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_PropertyUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintBodyAttachmentCustomization"

TSharedRef<IPropertyTypeCustomization> FAGX_ConstraintBodyAttachmentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ConstraintBodyAttachmentCustomization());
}

void FAGX_ConstraintBodyAttachmentCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	BodyAttachmentProperty = StructPropertyHandle;

#if AGX_UNREAL_RIGID_BODY_COMPONENT
	RigidBodyProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, RigidBodyComponent));
#else
	RigidBodyProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, RigidBodyActor));
#endif

	// Use default visualization in the name column (left side).
	HeaderRow.NameContent()[StructPropertyHandle->CreatePropertyNameWidget()];

	// Show the name of chosen rigid body in the value column (right side).
	HeaderRow.ValueContent()
		.MinDesiredWidth(250.0f) // from SPropertyEditorAsset::GetDesiredWidth
		.MaxDesiredWidth(
			0)[SNew(STextBlock)
				   .Text(this, &FAGX_ConstraintBodyAttachmentCustomization::GetRigidBodyLabel)
				   .Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				   .ColorAndOpacity(FLinearColor(1.0f, 0.45f, 0, 1.0f))
				   .MinDesiredWidth(250)];
}

void FAGX_ConstraintBodyAttachmentCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FrameDefiningActorProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, FrameDefiningActor));

	const UObject* FrameDefiningActor =
		FAGX_PropertyUtilities::GetObjectFromHandle(FrameDefiningActorProperty);

	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);

	// Use default visualization for most of the structs properties.
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		if (TSharedPtr<IPropertyHandle> ChildHandle =
				StructPropertyHandle->GetChildHandle(ChildIndex))
		{
			// Add default visualization.
			IDetailPropertyRow& DefaultPropertyRow =
				StructBuilder.AddProperty(ChildHandle.ToSharedRef());

			// Frame properties only visible if Rigid Body Actor has been set.
			if (!FAGX_PropertyUtilities::PropertyEquals(ChildHandle, RigidBodyProperty))
			{
				TAttribute<EVisibility> IsVisibleDelegate = TAttribute<EVisibility>::Create(
					TAttribute<EVisibility>::FGetter::CreateLambda([this] {
#if AGX_UNREAL_RIGID_BODY_COMPONENT
						return HasRigidBodyComponent() ? EVisibility::Visible : EVisibility::Collapsed;
#else
						return HasRigidBodyActor() ? EVisibility::Visible : EVisibility::Collapsed;
#endif
					}));

				DefaultPropertyRow.Visibility(IsVisibleDelegate);
			}

			// Add "Create New" option to context menu for the Frame Defining Actor.
			if (FAGX_PropertyUtilities::PropertyEquals(ChildHandle, FrameDefiningActorProperty))
			{
				// To do this, the default widgets need to be extracted, and the row needs
				// to be built up again using them, removing the additional Reset button,
				// and adding the additional context menu item.

				TSharedPtr<SWidget> DefaultNameWidget;
				TSharedPtr<SWidget> DefaultValueWidget;
				DefaultPropertyRow.GetDefaultWidgets(DefaultNameWidget, DefaultValueWidget, true);

				FAGX_SlateUtilities::RemoveChildWidgetByType(
					DefaultValueWidget, "SResetToDefaultPropertyEditor");

				FDetailWidgetRow& CustomPropertyRow =
					DefaultPropertyRow.CustomWidget(/*bShowChildren*/ true);

				CustomPropertyRow.AddCustomContextMenuAction(
					FUIAction(
						FExecuteAction::CreateSP(
							this, &FAGX_ConstraintBodyAttachmentCustomization::
									  CreateAndSetFrameDefiningActor),
						FCanExecuteAction::CreateLambda(
							[this] { return !HasFrameDefiningActor(); })),
					LOCTEXT("CreateFrameDefiningActorContextMenuItem", "Create New"));

				CustomPropertyRow.NameContent()[DefaultNameWidget.ToSharedRef()];

				CustomPropertyRow.ValueContent()
					.MinDesiredWidth(250.0f) // from SPropertyEditorAsset::GetDesiredWidth
					.MaxDesiredWidth(0)
						[SNew(SBox)
							 .VAlign(VAlign_Center)
							 .Padding(FMargin(
								 0, 0, 0,
								 0)) // Line up with the other properties due to having no reset
									 // to default button
								 [SNew(SVerticalBox) +
								  SVerticalBox::Slot().AutoHeight()
									  [SNew(SHorizontalBox) +
									   SHorizontalBox::Slot()[DefaultValueWidget.ToSharedRef()]]]];
			}
		}
	}

	StructBuilder.AddCustomRow(FText::FromString(""));
}

FText FAGX_ConstraintBodyAttachmentCustomization::GetRigidBodyLabel() const
{
#if AGX_UNREAL_RIGID_BODY_COMPONENT
	UObject* PropertyValue = FAGX_PropertyUtilities::GetObjectFromHandle(RigidBodyProperty);
	FComponentReference* ComponentReference = Cast<FComponentReference>(PropertyValue);
	USceneComponent* SceneComponent = ComponentReference->GetComponent(nullptr);
	UAGX_RigidBodyComponent* RigidBodyComponent = Cast<UAGX_RigidBodyComponent>(SceneComponent);
	FString RigidBodyName = RigidBodyComponent->GetName();
	return FText::FromString(TEXT("(") + RigidBodyName + TEXT(")"));
#else
	FString RigidBodyName;
	if (const AActor* RigidBody =
			Cast<AActor>(FAGX_PropertyUtilities::GetObjectFromHandle(RigidBodyProperty)))
	{
		RigidBodyName = "(" + RigidBody->GetActorLabel() + ")";
	}

	return FText::FromString(RigidBodyName);
#endif
}

#if AGX_UNREAL_RIGID_BODY_COMPONENT
bool FAGX_ConstraintBodyAttachmentCustomization::HasRigidBodyComponent() const
{
	/// \todo This may be the FComponentReference itself. Perhaps we need to
	/// reach into it and check if the reference is pointing to something.
	/// \todo Perhaps this whole GetObjectFromHandle setup only works with
	/// raw pointers and not a struct/class like FComponentReference.
	return FAGX_PropertyUtilities::GetObjectFromHandle(RigidBodyProperty) != nullptr;
}
#else
bool FAGX_ConstraintBodyAttachmentCustomization::HasRigidBodyActor() const
{
	return FAGX_PropertyUtilities::GetObjectFromHandle(RigidBodyProperty);
}
#endif

bool FAGX_ConstraintBodyAttachmentCustomization::HasFrameDefiningActor() const
{
	return FAGX_PropertyUtilities::GetObjectFromHandle(FrameDefiningActorProperty);
}

FString GenerateFrameDefiningActorName(
	const UAGX_ConstraintComponent* Constraint, const UAGX_RigidBodyComponent* RigidBody)
{
	check(Constraint);
	return "Constraint Frame Actor for " + Constraint->GetName();
}

void FAGX_ConstraintBodyAttachmentCustomization::CreateAndSetFrameDefiningActor()
{
	check(BodyAttachmentProperty);

	if (FAGX_PropertyUtilities::GetObjectFromHandle(FrameDefiningActorProperty))
	{
		return; // Already exists.
	}

	UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(
		FAGX_PropertyUtilities::GetParentObjectOfStruct(BodyAttachmentProperty));

	check(Constraint);

#if AGX_UNREAL_RIGID_BODY_COMPONENT
	/// \todo How do we get from the BodyAttachmentProperty to the Actor that owns it?
	AActor* ParentActor = nullptr;
#else
	AActor* ParentActor =
		Cast<AActor>(FAGX_PropertyUtilities::GetObjectFromHandle(RigidBodyProperty)); // optional
#endif

	// Create the new Constraint Frame Actor.
	AActor* NewActor = FAGX_EditorUtilities::CreateConstraintFrameActor(
		ParentActor,
		/*Select*/ true,
		/*ShowNotification*/ true,
		/*InPlayingWorldIfAvailable*/ true);

	// Set the new actor to our property.
#if 0
	// This should work, but doesn't! Using worked-around below instead...
	FPropertyAccess::Result Result = FrameDefiningActorProperty->SetValue((UObject*)NewActor);
	check(Result == FPropertyAccess::Success);
#else
	FAGX_ConstraintBodyAttachment* BodyAttachment =
		FAGX_PropertyUtilities::GetStructFromHandle<FAGX_ConstraintBodyAttachment>(
			BodyAttachmentProperty, Constraint);

	BodyAttachment->FrameDefiningActor = NewActor;
	BodyAttachment->OnFrameDefiningActorChanged(Constraint);
#endif
}

#undef LOCTEXT_NAMESPACE
