#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

#include "AGX_CollisionGroupManager.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

class AGXUNREAL_API FAGX_CollisionGroupManagerCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void OnConstraintTypeComboBoxChanged(
		TSharedPtr<FName> NewSelectedItem, ESelectInfo::Type InSeletionInfo,
		AAGX_CollisionGroupManager* CollisionGroupManager, FName* SelectedGroup);
};
