// Fill out your copyright notice in the Description page of Project Settings.

#include "AGX_ConstraintBodyAttachmentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "NotificationManager.h"
#include "SNotificationList.h"

#include "AGX_Constraint.h"
#include "AGX_ConstraintFrameActor.h"
#include "AGX_EditorUtilities.h"
#include "Constraints/AGX_Constraint.h"
#include "Utilities/AGX_SlateUtilities.h"


#define LOCTEXT_NAMESPACE "FAGX_ConstraintBodyAttachmentCustomization"


UObject*
GetObjectFromPropertyHandle(
	const TSharedPtr<IPropertyHandle> &PropertyHandle)
{
	if (PropertyHandle && 
		PropertyHandle->IsValidHandle() &&
		PropertyHandle->GetProperty() &&
		PropertyHandle->GetProperty()->IsA(UObjectProperty::StaticClass()))
	{
		UObject* Object = nullptr;
		if (PropertyHandle->GetValue(Object) != FPropertyAccess::Result::Fail)
		{
			return Object;
		}
	}
	return nullptr;
}

/**
 * Given a IPropertyHandle to a struct property (value-type) in a class,
 * returns a pointer to the actual struct data.
 */
template<typename TStruct, typename TOwningClass>
TStruct* GetStructFromHandle(const TSharedPtr<IPropertyHandle> &PropertyHandle, TOwningClass *OwningClass)
{
	if (PropertyHandle->IsValidHandle())
		return PropertyHandle->GetProperty()->ContainerPtrToValuePtr<TStruct>(OwningClass);
	else
		return nullptr;
}


bool
PropertyEquals(
	const TSharedPtr<IPropertyHandle> &First,
	const TSharedPtr<IPropertyHandle> &Second)
{
	return
		First->IsValidHandle() &&
		Second->IsValidHandle() &&
		First->GetProperty() == Second->GetProperty();
}


UObject*
GetParentObjectOfStruct(
	const TSharedPtr<IPropertyHandle> &StructPropertyHandle)
{
	TArray<UObject*> OuterObjects;
	StructPropertyHandle->GetOuterObjects(OuterObjects);

	return OuterObjects.Num() > 0 ? OuterObjects[0] : nullptr;
}


TSharedRef<IPropertyTypeCustomization>
FAGX_ConstraintBodyAttachmentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ConstraintBodyAttachmentCustomization());
}


void
FAGX_ConstraintBodyAttachmentCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	BodyAttachmentProperty = StructPropertyHandle;

	RigidBodyProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, RigidBodyActor));

	// Use default visualization in the name column (left side).
	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];

	// Show the name of chosen rigid body in the value column (right side).
	HeaderRow.ValueContent()
		.MinDesiredWidth(250.0f) // from SPropertyEditorAsset::GetDesiredWidth
		.MaxDesiredWidth(0)
		[
			SNew(STextBlock)
			.Text(this, &FAGX_ConstraintBodyAttachmentCustomization::GetRigidBodyLabel)
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.MinDesiredWidth(250)
		];
}


void
FAGX_ConstraintBodyAttachmentCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FrameDefiningActorProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, FrameDefiningActor));

	const UObject* FrameDefiningActor = GetObjectFromPropertyHandle(FrameDefiningActorProperty);


	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);

	// Use default visualization for most of the structs properties.
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		if (TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex))
		{
			// Add default visualization.
			IDetailPropertyRow& DefaultPropertyRow = StructBuilder.AddProperty(ChildHandle.ToSharedRef());

			// Frame properties only visible if Rigid Body Actor has been set.
			if (!PropertyEquals(ChildHandle, RigidBodyProperty))
			{
				TAttribute<EVisibility> IsVisibleDelegate = TAttribute<EVisibility>::Create(
					TAttribute<EVisibility>::FGetter::CreateLambda(
						[this] { return HasRigidBodyActor() ? EVisibility::Visible : EVisibility::Collapsed; }));

				DefaultPropertyRow.Visibility(IsVisibleDelegate);
			}
			
			// Add "Create New" option to context menu for the Frame Defining Actor.
			if (PropertyEquals(ChildHandle, FrameDefiningActorProperty))
			{
				// To do this, the default widgets need to be extracted, and the row needs
				// to be built up again using them, removing the additional Reset button,
				// and adding the additional context menu item.

				TSharedPtr<SWidget> DefaultNameWidget;
				TSharedPtr<SWidget> DefaultValueWidget;
				DefaultPropertyRow.GetDefaultWidgets(DefaultNameWidget, DefaultValueWidget, true);

				FAGX_SlateUtilities::RemoveChildWidgetByType(DefaultValueWidget, "SResetToDefaultPropertyEditor");
				
				FDetailWidgetRow &CustomPropertyRow = DefaultPropertyRow.CustomWidget(/*bShowChildren*/ true);
				
				CustomPropertyRow.AddCustomContextMenuAction(
					FUIAction(
						FExecuteAction::CreateSP(this, &FAGX_ConstraintBodyAttachmentCustomization::CreateAndSetFrameDefiningActor),
						FCanExecuteAction::CreateLambda([this]{ return !HasFrameDefiningActor(); })),
					LOCTEXT("CreateFrameDefiningActorContextMenuItem", "Create New"));

				CustomPropertyRow.NameContent()
					[
						DefaultNameWidget.ToSharedRef()
					];

				CustomPropertyRow.ValueContent()
					.MinDesiredWidth(250.0f) // from SPropertyEditorAsset::GetDesiredWidth
					.MaxDesiredWidth(0)
					[
						SNew(SBox)
						.VAlign(VAlign_Center)
						.Padding(FMargin(0, 0, 0, 0)) // Line up with the other properties due to having no reset to default button
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								[
									DefaultValueWidget.ToSharedRef()
								]
							]
						]
					];
			}
		}
	}

	StructBuilder.AddCustomRow(FText::FromString(""));
}


FText
FAGX_ConstraintBodyAttachmentCustomization::GetRigidBodyLabel() const
{
	FString RigidBodyName;
	if (const AActor* RigidBody = Cast<AActor>(GetObjectFromPropertyHandle(RigidBodyProperty)))
	{
		RigidBodyName = "(" + RigidBody->GetActorLabel() + ")";
	}

	return FText::FromString(RigidBodyName);
}


bool
FAGX_ConstraintBodyAttachmentCustomization::HasRigidBodyActor() const
{
	return GetObjectFromPropertyHandle(RigidBodyProperty);
}


bool
FAGX_ConstraintBodyAttachmentCustomization::HasFrameDefiningActor() const
{
	return GetObjectFromPropertyHandle(FrameDefiningActorProperty);
}


FString GenerateFrameDefiningActorName(const AAGX_Constraint* Constraint, const AActor* RigidBody)
{
	check(Constraint);
	return "Constraint Frame Actor for " + Constraint->GetName();
}

void
FAGX_ConstraintBodyAttachmentCustomization::CreateAndSetFrameDefiningActor()
{
	check(BodyAttachmentProperty);

	if (GetObjectFromPropertyHandle(FrameDefiningActorProperty))
		return; // already exists
	
	AAGX_Constraint* Constraint = Cast<AAGX_Constraint>(
		GetParentObjectOfStruct(BodyAttachmentProperty));

	check(Constraint);

	AActor* RigidBody = Cast<AActor>(
		GetObjectFromPropertyHandle(RigidBodyProperty)); // optional

	// Create the new Constraint Frame Actor.
	AActor* NewActor = FAGX_EditorUtilities::CreateConstraintFrameActor(
		RigidBody,
		/*Select*/ true,
		/*ShowNotification*/ true,
		/*InPlayingWorldIfAvailable*/ true);

	// Set the new actor to our property.
		
#if 0 // This should work, but doesn't! Using worked-around below instead...
	FPropertyAccess::Result Result = FrameDefiningActorProperty->SetValue((UObject*)NewActor);
	check(Result == FPropertyAccess::Success);
#else
	FAGX_ConstraintBodyAttachment* BodyAttachment =
		GetStructFromHandle<FAGX_ConstraintBodyAttachment>(
			BodyAttachmentProperty, Constraint);

	BodyAttachment->FrameDefiningActor = NewActor;

	BodyAttachment->OnFrameDefiningActorChanged(Constraint);
#endif
}

#undef LOCTEXT_NAMESPACE
