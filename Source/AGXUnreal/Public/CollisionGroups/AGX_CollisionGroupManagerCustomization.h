#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

#include "AGX_CollisionGroupManager.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Collision Group Manager object in the Editor.
 */
class AGXUNREAL_API FAGX_CollisionGroupManagerCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void OnComboBoxChanged(
		TSharedPtr<FName> NewSelectedItem, ESelectInfo::Type InSeletionInfo,
		AAGX_CollisionGroupManager* CollisionGroupManager, FName* SelectedGroup);

	void UpdateAvailableCollisionGroups(const AAGX_CollisionGroupManager* CollisionGroupManager);

private:
	TArray<TSharedPtr<FName>> AvailableCollisionGroups;
};
