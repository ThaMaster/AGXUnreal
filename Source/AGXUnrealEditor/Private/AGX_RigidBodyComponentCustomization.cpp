#include "AGX_RigidBodyComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_RigidBodyComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_RigidBodyComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_RigidBodyComponentCustomization);
}

void FAGX_RigidBodyComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_RigidBodyComponent* RigidBodyComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_RigidBodyComponent>(
			DetailBuilder);

	if (!RigidBodyComponent)
		return;

	RigidBodyComponent->OnComponentView();
}

#undef LOCTEXT_NAMESPACE
