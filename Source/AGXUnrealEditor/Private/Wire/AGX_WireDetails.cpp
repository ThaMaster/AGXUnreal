#include "Wire/AGX_WireDetails.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireDetailsRuntime.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "AGX_WireDetails"

TSharedRef<IDetailCustomization> FAGX_WireDetails::MakeInstance()
{
	return MakeShareable(new FAGX_WireDetails());
}

void FAGX_WireDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& RuntimeCategory = DetailBuilder.EditCategory("AGX Runtime");
	RuntimeCategory.AddCustomBuilder(
		MakeShareable(new FAGX_WireDetailsRuntime(DetailBuilder)));
}
