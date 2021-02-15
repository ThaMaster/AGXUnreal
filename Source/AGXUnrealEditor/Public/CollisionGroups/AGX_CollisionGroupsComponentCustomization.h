#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UAGX_CollisionGroupsComponent;

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Collision Group component in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_CollisionGroupsComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
