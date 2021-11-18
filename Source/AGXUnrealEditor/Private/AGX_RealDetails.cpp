#include "AGX_RealDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Real.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "EditorSupportDelegates.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "PropertyHandle.h"
#include "PropertyPathHelpers.h"
#include "ScopedTransaction.h"
#include "UObject/NameTypes.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/NumericTypeInterface.h"
#include "Widgets/Input/SSpinBox.h"

// System includes.
#include <limits>

#define LOCTEXT_NAMESPACE "FAGX_RealDetails"

TSharedRef<IPropertyTypeCustomization> FAGX_RealDetails::MakeInstance()
{
	return MakeShareable(new FAGX_RealDetails());
}

namespace AGX_RealDetails_helpers
{
	class FAGX_RealInterface : public INumericTypeInterface<double>
	{
	public:
		static FString StaticToString(const double& Value)
		{
			FString Result = FString::Printf(TEXT("%g"), Value);
			// UE_LOG(
			//	LogAGX, Warning, TEXT("FAGX_RealInterface converted double '%g' to string '%s'."),
			//	Value, *Result);
			return Result;
		}

		static TOptional<double> StaticFromString(const FString& InString)
		{
			TOptional<double> Result = FCString::Atod(*InString);
			// if (Result.IsSet())
			//{
			//	UE_LOG(
			//		LogAGX, Warning,
			//		TEXT("FAGX_RealInterface converted string '%s' to double '%g'."), *InString,
			//		Result.GetValue());
			// }
			// else
			//{
			//	UE_LOG(
			//		LogAGX, Warning,
			//		TEXT("FAGX_RealInterface tried to convert string '%s' to double, but Atod "
			//			 "failed."),
			//		*InString);
			// }
			return Result;
		}

		virtual FString ToString(const double& Value) const override
		{
			return StaticToString(Value);
		}

		virtual TOptional<double> FromString(
			const FString& InString, const double& /*InExistingValue*/) override
		{
			return StaticFromString(InString);
		}

		virtual bool IsCharacterValid(TCHAR InChar) const override
		{
			auto IsValidLocalizedCharacter = [InChar]() -> bool
			{
				const FDecimalNumberFormattingRules& NumberFormattingRules =
					ExpressionParser::GetLocalizedNumberFormattingRules();
				return InChar == NumberFormattingRules.GroupingSeparatorCharacter ||
					   InChar == NumberFormattingRules.DecimalSeparatorCharacter ||
					   Algo::Find(NumberFormattingRules.DigitCharacters, InChar) != 0;
			};

			static const FString ValidChars = TEXT("1234567890eE-+");
			return InChar != 0 &&
				   (ValidChars.GetCharArray().Contains(InChar) || IsValidLocalizedCharacter());
		}

		virtual int32 GetMinFractionalDigits() const
		{
			return 0;
		}

		virtual int32 GetMaxFractionalDigits() const
		{
			return 0;
		}

		virtual void SetMinFractionalDigits(const TAttribute<TOptional<int32>>& NewValue)
		{
		}

		virtual void SetMaxFractionalDigits(const TAttribute<TOptional<int32>>& NewValue)
		{
		}
	};
}

void FAGX_RealDetails::CustomizeHeader(
	TSharedRef<IPropertyHandle> InRealHandle, FDetailWidgetRow& InHeaderRow,
	IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	StructHandle = InRealHandle;
	ValueHandle = StructHandle->GetChildHandle(TEXT("Value"));

	ValueHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(SharedThis(this), &FAGX_RealDetails::OnValueChanged));

	// clang-format off
	InHeaderRow
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(StructHandle->GetPropertyDisplayName())
		.ToolTipText(StructHandle->GetToolTipText())
	]
	.ValueContent()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SSpinBox<double>)
			.TypeInterface(MakeShareable(new AGX_RealDetails_helpers::FAGX_RealInterface))
			.MinValue(0.0)
			.MinSliderValue(0.0)
			.MaxValue(std::numeric_limits<double>::max())
			.MaxSliderValue(1e10)
			.SliderExponent(1.0f)
			.OnValueChanged(this, &FAGX_RealDetails::OnSpinChanged)
			.OnValueCommitted(this, &FAGX_RealDetails::OnSpinCommitted)
			.Value(this, &FAGX_RealDetails::GetDoubleValue)
			.Visibility(this, &FAGX_RealDetails::VisibleWhenSingleSelection)
		]
		+ SOverlay::Slot()
		[
			SNew(SEditableText)
			.OnTextChanged(this, &FAGX_RealDetails::OnTextChanged)
			.OnTextCommitted(this, &FAGX_RealDetails::OnTextCommitted)
			.Text(this, &FAGX_RealDetails::GetTextValue)
			.OnTextCommitted(this, &FAGX_RealDetails::OnTextCommitted)
			.OnTextChanged(this, &FAGX_RealDetails::OnTextChanged)
			.Visibility(this, &FAGX_RealDetails::VisibleWhenMultiSelection)
		]
		+ SOverlay::Slot()
		[
			SNew(SEditableText)
			.IsReadOnly(true)
			.Text(LOCTEXT("NoSelection", "NoSelection"))
			.Visibility(this, &FAGX_RealDetails::VisibleWhenNoSelectionOrInvalidHandle)
		]
	];
	// clang-format on
}

void FAGX_RealDetails::CustomizeChildren(
	TSharedRef<IPropertyHandle> RealHandle, IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

EVisibility FAGX_RealDetails::VisibleWhenSingleSelection() const
{
	if (!StructHandle->IsValidHandle())
	{
		return EVisibility::Collapsed;
	}
	return FAGX_EditorUtilities::VisibleIf(StructHandle->GetNumPerObjectValues() == 1);
}

EVisibility FAGX_RealDetails::VisibleWhenMultiSelection() const
{
	if (!StructHandle->IsValidHandle())
	{
		return EVisibility::Collapsed;
	}
	return FAGX_EditorUtilities::VisibleIf(StructHandle->GetNumPerObjectValues() > 1);
}

EVisibility FAGX_RealDetails::VisibleWhenNoSelectionOrInvalidHandle() const
{
	if (!StructHandle->IsValidHandle())
	{
		return EVisibility::Collapsed;
	}
	return FAGX_EditorUtilities::VisibleIf(StructHandle->GetNumPerObjectValues() == 0);
}

double FAGX_RealDetails::GetDoubleValue() const
{
	if (!ValueHandle->IsValidHandle())
	{
		// The Spin Box will not be displayed while the handle is invalid, so
		// it doesn't matter what we return here.
		return -1.0;
	}
	double Value;
	FPropertyAccess::Result Status = ValueHandle->GetValue(Value);
	switch (Status)
	{
		case FPropertyAccess::Success:
			return Value;
		case FPropertyAccess::MultipleValues:
			// The Spin Box will not be displayed while there are multiple
			// objects selected, so it doesn't matter what we return here.
			return -2.0;
		case FPropertyAccess::Fail:
			// Can we do any error handling here?
			return -3.0;
	}
}

FText FAGX_RealDetails::GetTextValue() const
{
	if (!ValueHandle->IsValidHandle())
	{
		// The Editable Text will not be displayed while the handle is invalid,
		// so it doesn't matter what we return here.
		return LOCTEXT("", "");
	}
	double Value = 0.0;
	FPropertyAccess::Result Status = ValueHandle->GetValue(Value);
	switch (Status)
	{
		case FPropertyAccess::Success:
			return FText::FromString(FString::Printf(TEXT("%g"), Value));
		case FPropertyAccess::MultipleValues:
			// The Editable Text will not be displayed while there are multiple
			// objects selected, so it doesn't matter what we return here.
			return LOCTEXT("MultipleValues", "Multiple Values");
		case FPropertyAccess::Fail:
			// Can we do any error handling here?
			return LOCTEXT("CouldNotReadValue", "<could not read value>");
	}
}

void FAGX_RealDetails::OnSpinChanged(double NewValue)
{
	// UE_LOG(LogAGX, Warning, TEXT("OnSpinChanged: %g"), NewValue);

	/// @todo Consider doing input validation here, and make the background red if invalid.
}

void FAGX_RealDetails::OnSpinCommitted(double NewValue, ETextCommit::Type CommitInfo)
{
	// UE_LOG(LogAGX, Warning, TEXT("OnSpinCommitted: %g"), NewValue);
	if (!StructHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Spin value to AGX Real, the handle is invalid."));
		return;
	}

	FScopedTransaction Transaction(FText::Format(
		LOCTEXT("SpinTransaction", "Edit {0}"), StructHandle->GetPropertyDisplayName()));

#if 1
	// Implementation based on https://git.algoryx.se/algoryx/agxunreal/-/issues/499#note_98909. The
	// idea is to use IPropertyHandle::GeneratePathToProperty and repeated calls to
	// IPropertyHandle::GetChildHandle to generate a chain from the UObject to the leaf FProperty,
	// at which point we will have something we can write to, even for template instances.
	//
	// The instruction:
	// > In that case, you can probably call IPropertyHandle::GeneratePathToProperty(), split that,
	// > and then get your FProperty* chain from repeatedly calling
	// > IPropertyHandle::GetChildHandle() - that method can deal with array indices as well, in
	// > case you ever want to have these in containers.

	FString PathToValue = ValueHandle->GeneratePathToProperty();
	UE_LOG(LogAGX, Warning, TEXT("Path to value: '%s'."), *PathToValue);

	// Consider creating a FCachedPropertyPath here. Is that shareable between objects, or specific
	// to a particular instance? Does it matter if the two objects is of the same class or not?

	TArray<UObject*> SelectedObjects;
	ValueHandle->GetOuterObjects(SelectedObjects);

	/// @todo PreEditChange here.

	// Store the current values so that we can compare them against values in template instances.
	TArray<double> OldValues;
	OldValues.Reserve(SelectedObjects.Num());
	for (UObject* SelectedObject : SelectedObjects)
	{
		double OldValue;
		PropertyPathHelpers::GetPropertyValue(SelectedObject, PathToValue, OldValue);
		OldValues.Add(OldValue);
	}

	for (UObject* SelectedObject : SelectedObjects)
	{
		PropertyPathHelpers::SetPropertyValue(SelectedObject, PathToValue, NewValue);
	}

	for (int32 I = 0; I < SelectedObjects.Num(); ++I)
	{
		UObject* SelectedObject = SelectedObjects[I];
		if (!SelectedObject->IsTemplate())
		{
			continue;
		}

		double OldValue = OldValues[I];

		TArray<UObject*> Instances;
		SelectedObject->GetArchetypeInstances(Instances);
		for (UObject* Instance : Instances)
		{
			double CurrentValue;
			PropertyPathHelpers::GetPropertyValue(Instance, PathToValue, CurrentValue);
			if (CurrentValue == OldValue)
			{
				PropertyPathHelpers::SetPropertyValue(Instance, PathToValue, NewValue);
			}
		}
	}

	/// @todo PostEditChangeChainProperty here.

	/// @tod Archetype instances here.

	// UE_LOG(LogAGX, Warning, )
	// IPropertyHandle* HandleIt = ValueHandle.Get();
	// while (HandleIt != nullptr)
	// {
	//	UE_LOG(LogAGX, Warning, )
	// }
	// ValueHandle
	//	->GetParentHandle()
#endif

// Write the Property using IPropertyHandle::SetValue. This is the way we are supposed to do it.
#if 0
	// Use IPropertyHandle::SetValue to set the new value on the selected objects and the instances.
	// This doesn't work because it does the equivalent of
	//   MyDoubleProperty = Atod(ParseFloat(Printf("%f", NewValue)));
	// in IPropertyHandle::SetValue and FPropertyValueImpl::ImportText, which destroys small values.
	// If this had worked then we wouldn't need FAGX_Real all, a regular double would have worked.
	FPropertyAccess::Result AccessResult = ValueHandle->SetValue(NewValue);
	if (AccessResult == FPropertyAccess::Fail)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Spin value to AGX Real, the handle failed to set the new "
				 "value."));
	}
#endif

// Write through IPropertyHandle::SetValueFromFormattedString. The intention is that by formatting
// the string ourselves we won't have to suffer from the destructive Atod(Printf("%f")) round-trip
// that Unreal Engine otherwise does.
#if 0
	// This doesn't work. The edited object is updated but not instances. It does the equivalent of
	// the following in FPropertyNode::PropagatePropertyChange:
	//
	//  void WriteIfCurrentSameAsOld(
	//      UObject* Instance,
	//      FString NewValue = "1e-11",
	//      double PreviousValue = 1e-10)
	//  {
	//      FString PreviousValueStr = Printf("%f", PreviousValue);  // "0.000000".
	//      double Temp = Atod(PreviousValue); // 0.0.
	//      if (Instance->MyDoubleProperty == Temp) // 1e-10 == 0.0.
	//      {
	//          Instance->MyDoubleProperty = Atod(NewValue); // Never get here.
	//      }
	//  }
	//
	// Thus, the instance is never updated because before the equality test the template's
	// PreviousValue takes a round trip to a string, destroying the value in the process, before
	// being compared with the value in the instance. So setting any value too small for "%f" on
	// the template causes any instance with a non-zero value, even if equal to the template's old
	// value, to not be updated.
	FString NewValueAsString =
		AGX_RealDetails_helpers::FAGX_RealInterface::StaticToString(NewValue);
	FPropertyAccess::Result AccessResult = ValueHandle->SetValueFromFormattedString(
		NewValueAsString, EPropertyValueSetFlags::InteractiveChange);
	if (AccessResult == FPropertyAccess::Fail)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Spin value to AGX Real, the handle failed to set the new "
				 "value."));
	}
#endif

// This attempt want to use FComponentEditorUtils::PropagateDefaultValueChange for the propagation
// to the template instances, just like FComponentTransformDetails does. Unfortunately, we can't
// pass our Value FProperty or FAGX_Real FProperty to it because those doesn't have a UClass Owner
// so Unreal Editor crashes with a failed assert. I tried to walk up the Owner chain to find an
// FProperty that we are allowed to pass, but not able to find any.
#if 0
	FProperty* ValueProperty = ValueHandle->GetProperty();

	TArray<UObject*> SelectedObjects;
	ValueHandle->GetOuterObjects(SelectedObjects);

	// Let the selected objects know that we're about to modify them.
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PreEditChange(ValueProperty);
	}

	// Do the modification, but save the old value for each selected object so that
	// FComponentEditorUtils::PropagateDefaultValueChange can do the should-I-write check later.
	TArray<double> OldValues;
	OldValues.Reserve(SelectedObjects.Num());
	TArray<void*> RawData;
	ValueHandle->AccessRawData(RawData);
	for (void* RawDatum : RawData)
	{
		double* RawDouble = static_cast<double*>(RawDatum);
		OldValues.Add(*RawDouble);
		*RawDouble = NewValue;
	}

	// Let the selected objects know that we're done modifying them.
	FPropertyChangedEvent ChangedEvent(ValueProperty, EPropertyChangeType::ValueSet);
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PostEditChangeProperty(ChangedEvent);
	}

	check(SelectedObjects.Num() == OldValues.Num());

	// This was the first attempt at finding a propagatable FProperty, i.e. one that can be passed
	// to FComponentEditorUtils::PropagateDefaultValueChange. GetOwnerProperty will step up the
	// outer chain until the last FProperty owner is reached. We don't know what the next owner will
	// be, but it might be a UClass. Hopefully.
	//
#if 0
	FProperty* PropagatableProperty = ValueProperty->GetOwnerProperty();
#endif
	//
	// The above didn't work. PropagatableProperty also doesn't have a UClass Owner so
	// FComponentEditorUtils::PropagateDefaultValueChange crashes just like when passing
	// ValueProperty.

	// Trying to step up the owner chain until we find something that does have a UClass as its
	// owner. I had hoped to be able to find something useful, but alas, I find nothing of value.
	FFieldVariant FieldOrObject = ValueProperty->Owner;
	while (!FieldOrObject.IsUObject() && FieldOrObject.ToField() != nullptr &&
		   FieldOrObject.ToField()->GetOwner<UClass>() == nullptr)
	{
		FieldOrObject = FieldOrObject.ToField()->GetOwnerVariant();
	}
	if (FieldOrObject.ToField() == nullptr)
	{
		// We get here when modifying an FAGX_Real owned by an UAGX_ConstraintComponent. Don't know
		// how to continue from here, so bail.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any Field that has a UClass parent. PropagateDefaultValueChange "
				 "will fail."));
		return;
	}
	FProperty* PropagatableProperty = FieldOrObject.Get<FProperty>();
	if (PropagatableProperty == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find an FProperty that has a UClass parent. PropagateDefaultValueChange "
				 "will fail."));
		return;
	}

	// I don't know what we should do with OldValue now. OldValue is just a double, I have no idea
	// what PropagatableProperty will be a property for, or what value it has, if we can find one
	// at all.
	for (int32 I = 0; I < SelectedObjects.Num(); ++I)
	{
		UObject* SelectedObject = SelectedObjects[I];
		double OldValue = OldValues[I];

		// What if the FAGX_Real isn't even in a USceneComponent
		// at all? It could be in any type of UObject.
		USceneComponent* Scene = Cast<USceneComponent>(SelectedObject);
		TSet<USceneComponent*> UpdatedInstances;
		FComponentEditorUtils::PropagateDefaultValueChange(
			Scene, PropagatableProperty, OldValue, NewValue, UpdatedInstances);
	}
#endif

// This is an attempt at writing through IPropertyHandle::AccessRawData for the selected objects
// and to let PostEditChangeChainProperty handle updating any template instances. I don't think this
// will work because FComponentTransformDetails::OnSetTransform in ComponentTransformDetails.cpp
// does both PostEditChangeChainProperty and PostEditChangeChainProperty. Still, gotta try.
//
// It didn't work. PostEditChangeChainProperty doesn't do anything with the Property values, it only
// calls itself recursively on other objects.
#if 0
	FProperty* StructProperty = StructHandle->GetProperty();
	FProperty* ValueProperty = ValueHandle->GetProperty();

	TArray<UObject*> SelectedObjects;
	ValueHandle->GetOuterObjects(SelectedObjects);

	// Let all the selected objects know that we're about to modify them. This will add them to the
	// current undo/redo transaction.
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PreEditChange(ValueProperty);
	}

	// Write the new value to each selected object.
	TArray<void*> PropertyData;
	ValueHandle->AccessRawData(PropertyData);
	for (void* PropertyDatum : PropertyData)
	{
		double* RawProperty = static_cast<double*>(PropertyDatum);
		*RawProperty = NewValue;
	}

	// Let all the selected objects know that we're done modifying them.
	FPropertyChangedEvent PropertyChangedEvent(
		ValueProperty, EPropertyChangeType::ValueSet, MakeArrayView(SelectedObjects));
	FEditPropertyChain PropertyChain;
	//PropertyChain.AddHead(ValueProperty); // The double Value in FAGX_Real.
	PropertyChain.AddHead(StructProperty); // The FAGX_Real in FAGX_ConstraintDoublePropertyPerDof

// The Property we are editing can be arbitrarily nested inside other struct Properties.
// Must we build a complete chain with all of these? How? Can we use any of the two functions
// below?
#if 0
	StructHandle->GetParentHandle();
	StructProperty->GetOwnerProperty();
#endif
	PropertyChain.AddHead(StructHandle->GetParentHandle()->GetProperty()); // The PropertyPerDof in Constraint.

	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PostEditChangeChainProperty(PropertyChangedChainEvent);
	}
#endif

// Write each object, including template instances, through the facilities provided by FProperty.
// I think this should work, but crashes inside Unreal Engine code with a failed assert.
//   Assertion failed:
//   GetOwner<UClass>()
//   File:Runtime/CoreUObject/Public/UObject/UnrealType.h
//   Line: 376
// This is inside FProperty::ContainerUObjectPtrToValuePtrInternal.
//
// I suspect this is because our Property is nested inside (a) struct(s), it is not a Property
// directly on a UObject. I don't know what to do about this. The IPropertyHandle code uses
// FPropertyNode::GetValueBaseAddressFromObject. This is private code that I don't think I can call.
//
#if 0
	FProperty* Property = StructHandle->GetProperty();
	// The above line used Value handle first, with the appropriate usage below, but I changed to
	// StructHandle in an attempt at fixing the crash. My reasoning was that perhaps
	// FProperty::ContainerPtrToValuePtr doesn't work with Properties nested inside structs.
	// 'Value' here is the `double` named Value inside the struct FAGX_Real, and 'Struct' is the
	// FAGX_Real struct. It doesn't matter which handle we use because the FAGX_Real struct itself
	// can be a member of another struct. The constraint parameters (elasticity, damping, etc), for
	// example.

	TArray<UObject*> SelectedObjects;
	StructHandle->GetOuterObjects(SelectedObjects);

	// Let all the selected objects know that we're about to modify them. This will add them to the
	// current undo/redo transaction.
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PreEditChange(Property);
	}

	// Write the new value to each selected object, and store the old values so that we can compare
	// with template instances.
	const TArray<double> OldValues = [this, NewValue]()
	{
		TArray<void*> PropertyData;
		StructHandle->AccessRawData(PropertyData);
		TArray<double> OldValues;
		OldValues.Reserve(PropertyData.Num());
		for (void* PropertyDatum : PropertyData)
		{
			FAGX_Real* RawProperty = static_cast<FAGX_Real*>(PropertyDatum);
			OldValues.Add(RawProperty->Value);
			RawProperty->Value = NewValue;
		}
		return OldValues;
	}();

	// We assume that GetOuterObjects and AccessRawData return order-compatible
	// arrays, meaning that GetOuterObjects[I] owns AccessRawData[I].
	check(SelectedObjects.Num() == OldValues.Num());

	// Let all the selected objects know that we're done modifying them.
	FPropertyChangedEvent ChangedEvent(Property, EPropertyChangeType::ValueSet);
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PostEditChangeProperty(ChangedEvent);
	}

	// If any of the selected objects is a template type, then also update all matching instances of
	// the template. Matching meaning that the instance's value is the same as the selected object's
	// old value.
	for (int32 I = 0; I < SelectedObjects.Num(); ++I)
	{
		UObject* SelectedObject = SelectedObjects[I];
		if (!SelectedObject->IsTemplate())
		{
			continue;
		}

		double OldValue = OldValues[I];
		TArray<UObject*> Instances;
		SelectedObject->GetArchetypeInstances(Instances);
		for (UObject* Instance : Instances)
		{
			FAGX_Real* RawProperty = Property->ContainerPtrToValuePtr<FAGX_Real>(Instance);
			if (RawProperty->Value == OldValue)
			{
				Instance->PreEditChange(Property);
				RawProperty->Value = NewValue;
				Instance->PostEditChangeProperty(ChangedEvent);
			}
		}
	}
#endif

	// Let the various parts of the editor know about the change. I have no idea what I'm
	// supposed to call here.
	FEditorSupportDelegates::RedrawAllViewports.Broadcast();
	FEditorSupportDelegates::RefreshPropertyWindows.Broadcast();
}

void FAGX_RealDetails::OnTextChanged(const FText& NewText)
{
	// UE_LOG(LogAGX, Warning, TEXT("OnTextChanged"));

	/// @todo Consider doing input validation here, and make the background red if invalid.
}

void FAGX_RealDetails::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	// UE_LOG(LogAGX, Warning, TEXT("OnTextCommitted"));
	if (!ValueHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Text value to AGX Real, the handle is invalid."));
		return;
	}

	TOptional<double> NewValue =
		AGX_RealDetails_helpers::FAGX_RealInterface::StaticFromString(NewText.ToString());
	if (!NewValue.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Text value to AGX Real, '%s' is not a valid double."),
			*NewText.ToString());
		return;
	}
	FString NewValueAsString =
		AGX_RealDetails_helpers::FAGX_RealInterface::StaticToString(NewValue.GetValue());
	FPropertyAccess::Result AccessResult = ValueHandle->SetValueFromFormattedString(
		NewValueAsString, EPropertyValueSetFlags::InteractiveChange);
	switch (AccessResult)
	{
		case FPropertyAccess::Success:
			// All good.
			// UE_LOG(
			//	LogAGX, Warning, TEXT("New value %g committed to AGX Real value."),
			//	NewValue.GetValue());
			break;
		case FPropertyAccess::MultipleValues:
			// All good, I think.
			// UE_LOG(
			//	LogAGX, Warning, TEXT("New value %g committed to multiple AGX Real values."),
			//	NewValue.GetValue());
			break;
		case FPropertyAccess::Fail:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot commit new Text value to AGX Real, the handle failed to set the new "
					 "value."));
			break;
	}
}

void FAGX_RealDetails::OnValueChanged()
{
#if 1
	// Do we need to do anything here?
	return;
#else
	if (!ValueHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_RealDetails::OnValueChanged called without a valid handle. Surprising"));
		return;
	}

	double Value;
	FPropertyAccess::Result AccessStatus = ValueHandle->GetValue(Value);
	switch (AccessStatus)
	{
		case FPropertyAccess::Success:
			UE_LOG(LogAGX, Warning, TEXT("AGX_Real Property changed to %g."), Value);
			break;
		case FPropertyAccess::MultipleValues:
			UE_LOG(LogAGX, Warning, TEXT("AGX_Real Property changed to multiple values."), Value);
			break;
		case FPropertyAccess::Fail:
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX_Real Property changed but failed to read the new value."), Value);
			break;
	}
#endif
}

#undef LOCTEXT_NAMESPACE
