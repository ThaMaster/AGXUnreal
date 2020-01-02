#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UAGX_CollisionGroups;

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

class AGXUNREAL_API FAGX_CollisionGroupsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
