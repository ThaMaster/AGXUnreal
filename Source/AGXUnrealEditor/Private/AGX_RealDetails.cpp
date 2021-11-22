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
#include "UObject/Class.h"
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
	if (!StructHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot commit new Spin value to AGX Real, the handle is invalid."));
		return;
	}

	const FString ValuePath = ValueHandle->GeneratePathToProperty();
#if 0
	/// @todo Learn when we are allowed to cache the value path and when not.
	/// Passing it both to GetPropertyValue and SetPropertyValue for both the selected objects and
	/// template instances causes a crash due to a failed assert.
	///     LogCore: Assertion failed:
	///     InContainer == InPropertyPath.GetCachedContainer()
	///     File:Runtime/PropertyPath/Public/PropertyPathHelpers.h
	///     Line: 354
	/// I assume that means the cached path was used in a context where it shouldn't.
	/// Worst-case scenario is that a cached path is only valid for a single UObject, which makes
	/// it mostly usesless here.
	FCachedPropertyPath CachedValuePath(ValuePath);
#endif

	FProperty* ValueProperty = ValueHandle->GetProperty();

	// Get the selected objects. These are the objects that we should manipulate directly.
	TArray<UObject*> SelectedObjects;
	ValueHandle->GetOuterObjects(SelectedObjects);
	if (SelectedObjects.Num() == 0)
	{
		// If nothing is selected then nothing will be changed, so we don't need to do anything.
		return;
	}

	// This is a user-level interaction, so start a new undo/redo transaction. Any object that we
	// call Modify or PreEditChange on until this object goes out of scope will be included in a
	// single undo/redo step.
	FScopedTransaction Transaction(FText::Format(
		LOCTEXT("SpinTransaction", "Edit {0}"), StructHandle->GetPropertyDisplayName()));

	// Let the selected objects know that we are about to modify them, which will include them in
	// the current undo/redo transaction.
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PreEditChange(ValueProperty);
	}

	// Store the current values in the selected objects so that we can compare them against values
	// in template instances later.
	TArray<double> OldValues;
	OldValues.Reserve(SelectedObjects.Num());
	for (UObject* SelectedObject : SelectedObjects)
	{
		double OldValue;
		const bool bOldValueValid =
			PropertyPathHelpers::GetPropertyValue(SelectedObject, ValuePath, OldValue);
		if (!bOldValueValid)
		{
			/// @todo Is there something better we can do here?
			UE_LOG(
				LogAGX, Error, TEXT("Could not current '%s' value from '%s'. Assuming 0.0."),
				*ValuePath, *SelectedObject->GetName());
			OldValue = 0.0;
		}
		OldValues.Add(OldValue);
	}

	// Write the new value to each selected object.
	for (UObject* SelectedObject : SelectedObjects)
	{
		const bool bNewValueSet =
			PropertyPathHelpers::SetPropertyValue(SelectedObject, ValuePath, NewValue);
		if (!bNewValueSet)
		{
			/// @todo Is there something better we can do here?
			UE_LOG(
				LogAGX, Error, TEXT("Could not set '%s' on '%s'. Value remain unchanged."),
				*ValuePath, *SelectedObject->GetName());
		}
	}

	// Let selected objects know that we are done modifying them.
	FPropertyChangedEvent ChangedEvent(
		ValueProperty, EPropertyChangeType::ValueSet, MakeArrayView(SelectedObjects));
	FEditPropertyChain PropertyChain;
	TArray<FString> PropertyChainNames;
	ValuePath.ParseIntoArray(PropertyChainNames, TEXT("."));
	// What will happen if we have objects of different classes selected?
	UStruct* OuterClass = SelectedObjects[0]->GetClass();
	for (const FString& PropertyChainName : PropertyChainNames)
	{
		// Build a list of Properties along the ValuePath from one of the UObjects down to the
		// double inside the FAGX_Real. Each step need to know the type that the next Property
		// is inside in order to find it, so we keep OuterClass between loop iterations.
		//
		// If OuterClass is nullptr then that means that we hit an intermediate Property that wasn't
		// a struct, which is an unsupported case for now.
		check(OuterClass);
		if (OuterClass == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("When constructing Property chain for '%s', could not determine outer class "
					 "for '%s'. Property changed callbacks may not be triggered correctly."),
				*ValuePath, *PropertyChainName);
			/// @todo Unclear what we should do here. We can chose to just break and let the rest of
			/// the function continue executing. This will cause an incomplete path to be passed to
			/// PostEditChangeChainProperty. Better than nothing, I guess, since we've already
			/// modified the value, but any logic that depend on the path being correct will not
			/// trigger in response to the new value. That's bad. Should we generate this path first
			/// and do nothing if we fail?
			break;
		}

		FName Name(*PropertyChainName);
		FFieldVariant NextField = FindFProperty<FProperty>(OuterClass, Name);
		FProperty* NextProperty = NextField.Get<FProperty>();
		if (NextProperty == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Got no Property for '%s' in '%s'."), *PropertyChainName,
				*ValuePath);
			/// @todo Unclear what we should do here. We can chose to just break and let the rest of
			/// the function continue executing. This will cause an incomplete path to be passed to
			/// PostEditChangeChainProperty. Better than nothing, I guess, since we've already
			/// modified the value, but any logic that depend on the path being correct will not
			/// trigger in response to the new value. That's bad. Should we generate this path first
			/// and do nothing if we fail?
			break;
		}

		PropertyChain.AddTail(NextProperty);

		if (FStructProperty* StructProp = CastField<FStructProperty>(NextProperty))
		{
			OuterClass = StructProp->Struct;
		}
		else
		{
			// This better be the very last link in the chain. Otherwise, the next iteration of this
			// loop will terminate prematurely with an error message.
			OuterClass = nullptr;
		}
	}

	FPropertyChangedChainEvent ChainEvent(PropertyChain, ChangedEvent);
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PostEditChangeChainProperty(ChainEvent);
	}

	// For any selected template, propagate the change to all template instances that has the same
	// value as the selected object had.
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
			PropertyPathHelpers::GetPropertyValue(Instance, ValuePath, CurrentValue);
			if (CurrentValue == OldValue)
			{
				Instance->PreEditChange(ValueProperty);
				PropertyPathHelpers::SetPropertyValue(Instance, ValuePath, NewValue);
				Instance->PostEditChangeChainProperty(ChainEvent);
			}
		}
	}

	// Let the various parts of the editor know about the change. I have no idea what we're
	// supposed to call here. Is anything missing? Should something be removed?
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
