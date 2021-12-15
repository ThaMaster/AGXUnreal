// Copyright 2021, Algoryx Simulation AB.


#include "AGX_RigidBodyComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_RigidBodyComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_RigidBodyComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_RigidBodyComponentCustomization);
}

void FAGX_RigidBodyComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_RigidBodyComponent* RigidBodyComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_RigidBodyComponent>(
			*DetailBuilder);

	if (!RigidBodyComponent)
		return;

	RigidBodyComponent->OnComponentView();

	// clang-format off

	IDetailCategoryBuilder& Runtime =
		DetailBuilder->EditCategory(TEXT("AGX Runtime"), LOCTEXT("AGXRuntime", "AGX Runtime"));

	FDetailWidgetRow& HasNativeRow = Runtime.AddCustomRow(LOCTEXT("HasNative", "HasNative"));
	HasNativeRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("HasNative", "Has Native"))
	]
	.ValueContent()
	[
		SNew(STextBlock)
		.Text(this, &FAGX_RigidBodyComponentCustomization::GetHasNativeText)
	];

	// clang-format on
}

FText FAGX_RigidBodyComponentCustomization::GetHasNativeText() const
{
	if (DetailBuilder == nullptr)
	{
		return LOCTEXT("NoDetailBuilder", "No DetailBuilder, cannot get object being customized.");
	}

	UAGX_RigidBodyComponent* Body =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_RigidBodyComponent>(
			*DetailBuilder);

	if (Body == nullptr)
	{
		return LOCTEXT("(no body)", "(no body)");
	}

	return Body->HasNative() ? LOCTEXT("true", "true") : LOCTEXT("false", "false");
}

#undef LOCTEXT_NAMESPACE
