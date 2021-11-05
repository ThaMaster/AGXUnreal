#include "AGX_RealDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "EditorSupportDelegates.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "PropertyHandle.h"
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
			//UE_LOG(
			//	LogAGX, Warning, TEXT("FAGX_RealInterface converted double '%g' to string '%s'."),
			//	Value, *Result);
			return Result;
		}

		static TOptional<double> StaticFromString(const FString& InString)
		{
			TOptional<double> Result = FCString::Atod(*InString);
			//if (Result.IsSet())
			//{
			//	UE_LOG(
			//		LogAGX, Warning,
			//		TEXT("FAGX_RealInterface converted string '%s' to double '%g'."), *InString,
			//		Result.GetValue());
			//}
			//else
			//{
			//	UE_LOG(
			//		LogAGX, Warning,
			//		TEXT("FAGX_RealInterface tried to convert string '%s' to double, but Atod "
			//			 "failed."),
			//		*InString);
			//}
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
			.MaxValue(std::numeric_limits<double>::max())
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
	//UE_LOG(LogAGX, Warning, TEXT("OnSpinChanged: %g"), NewValue);

	/// @todo Consider doing input validation here, and make the background red if invalid.
}

void FAGX_RealDetails::OnSpinCommitted(double NewValue, ETextCommit::Type CommitInfo)
{
	//UE_LOG(LogAGX, Warning, TEXT("OnSpinCommitted: %g"), NewValue);
	if (!ValueHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Spin value to AGX Real, the handle is invalid."));
		return;
	}

#if 1
	TArray<UObject*> Objects;
	ValueHandle->GetOuterObjects(Objects);
	FProperty* Property = ValueHandle->GetProperty();

	for (UObject* Object : Objects)
	{
		Object->PreEditChange(Property);
	}

	TArray<double> OldValues;
	TArray<void*> PropertyData;
	ValueHandle->AccessRawData(PropertyData);
	for (void* PropertyDatum : PropertyData)
	{
		double* RawProperty = static_cast<double*>(PropertyDatum);
		OldValues.Add(*RawProperty);
		*RawProperty = NewValue;
	}

	check(Objects.Num() == OldValues.Num());
	check(Objects.Num() == PropertyData.Num());

	FPropertyChangedEvent ChangedEvent(Property);
	for (int32 I = 0; I < Objects.Num(); ++I)
	{
		UObject* Object = Objects[I];
		Object->PostEditChangeProperty(ChangedEvent);
		if (Object->IsTemplate())
		{
			/// @todo What about all the UObjects that aren't USceneComponent?
			if (USceneComponent* AsScene = Cast<USceneComponent>(Object))
			{
				double OldValue = OldValues[I];
				TSet<USceneComponent*> UpdatedInstances;
				FComponentEditorUtils::PropagateDefaultValueChange(AsScene, Property, OldValue, NewValue, UpdatedInstances);
			}
		}
	}

	FEditorSupportDelegates::RedrawAllViewports.Broadcast();
	FEditorSupportDelegates::RefreshPropertyWindows.Broadcast();

#elif 0
	// This doesn't work because it does
	//   MyDoubleProperty = ParseFloat(Printf("%f", NewValue));
	FPropertyAccess::Result AccessResult = ValueHandle->SetValue(NewValue);
#elif 0
	// This doesn't work. The edited object is updated but not instances. It does
	//  void WriteIfCurrentSameAsOld(
	//      UObject* Template, UObject* Instance, FString NewValue = "1e-11", double PreviousValue = 1e-10)
	//  {
	//      FString PreviousValueStr = FloatToString(PreviousValue);  // "0.000000".
	//      double Temp = ParseFloat(PreviousValue); // 0.0.
	//      if (Instance->MyDoubleProperty == Temp) // 1e-10 == 0.0.
	//      {
	//          Instance->MyDoubleProperty = StringToDouble(NewValue); 1e-11 = StringToDouble("1e-11").
	//      }
	//  }
	FString NewValueAsString =
		AGX_RealDetails_helpers::FAGX_RealInterface::StaticToString(NewValue);
	FPropertyAccess::Result AccessResult = ValueHandle->SetValueFromFormattedString(
		NewValueAsString, EPropertyValueSetFlags::InteractiveChange);
#endif

// Make sure this is enable when using any of the ValueHandle->SetValue.+ variants.
#if 0
	switch (AccessResult)
	{
		case FPropertyAccess::Success:
			// All good.
			//UE_LOG(LogAGX, Warning, TEXT("New value %g committed to AGX Real value."), NewValue);
			break;
		case FPropertyAccess::MultipleValues:
			// All good, I think.
			//UE_LOG(
			//	LogAGX, Warning, TEXT("New value %g committed to multiple AGX Real values."),
			//	NewValue);
			break;
		case FPropertyAccess::Fail:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot commit new Spin value to AGX Real, the handle failed to set the new "
					 "value."));
			break;
	}
#endif
}

void FAGX_RealDetails::OnTextChanged(const FText& NewText)
{
	//UE_LOG(LogAGX, Warning, TEXT("OnTextChanged"));

	/// @todo Consider doing input validation here, and make the background red if invalid.
}

void FAGX_RealDetails::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	//UE_LOG(LogAGX, Warning, TEXT("OnTextCommitted"));
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
			//UE_LOG(
			//	LogAGX, Warning, TEXT("New value %g committed to AGX Real value."),
			//	NewValue.GetValue());
			break;
		case FPropertyAccess::MultipleValues:
			// All good, I think.
			//UE_LOG(
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
