#include "AGX_RealDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "PropertyHandle.h"
#include "UObject/NameTypes.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_RealDetails"

TSharedRef<IPropertyTypeCustomization> FAGX_RealDetails::MakeInstance()
{
	return MakeShareable(new FAGX_RealDetails());
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
		SNew(STextBlock)
		.Text(LOCTEXT("<placeholder>", "<placeholder>"))
	];
	// clang-format on
}

void FAGX_RealDetails::CustomizeChildren(
	TSharedRef<IPropertyHandle> RealHandle, IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

void FAGX_RealDetails::OnValueChanged()
{
	UE_LOG(LogAGX, Warning, TEXT("AGX_Real changed"));
}

#undef LOCTEXT_NAMESPACE
