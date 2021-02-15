#pragma once

// AGX Dynamics for Unreal includes.
#include "CollisionGroups/AGX_CollisionGroupManager.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Types/SlateEnums.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Collision Group Manager object in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_CollisionGroupManagerCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void OnComboBoxChanged(
		TSharedPtr<FName> NewSelectedItem, ESelectInfo::Type InSeletionInfo,
		AAGX_CollisionGroupManager* CollisionGroupManager, FName* SelectedGroup);

	void UpdateAvailableCollisionGroups(const AAGX_CollisionGroupManager* CollisionGroupManager);

	void AddComboBox(
		IDetailCategoryBuilder& CategoryBuilder, AAGX_CollisionGroupManager* CollisionGroupManager,
		FText Name, FName* SelectedGroup);

private:
	TArray<TSharedPtr<FName>> AvailableCollisionGroups;
};
