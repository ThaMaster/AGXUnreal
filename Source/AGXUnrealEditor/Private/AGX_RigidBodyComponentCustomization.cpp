#include "AGX_RigidBodyComponentCustomization.h"

// AGXUnreal includes.
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

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX Dynamics");

	// Force the bTransformRootComponent flag to false in case there are several RigidBodyComponents
	// in the owning actor.
	if (RigidBodyComponent->bTransformRootComponent &&
		!RigidBodyComponent->TransformRootComponentAllowed())
	{
		RigidBodyComponent->bTransformRootComponent = false;
	}
}

#undef LOCTEXT_NAMESPACE
