// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Only for internal testing, do not merge!
 */
class AGXUNREALEDITOR_API FAGX_BrickAssetCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

private:
	FReply OnImportButtonClicked();
	FReply OnCreateBlueprintsButtonClicked();
	FReply OnUpdateBlueprintButtonClicked();
	FReply OnCopyFromOtherBlueprintButtonClicked();

	IDetailLayoutBuilder* DetailBuilder {nullptr};
	UBlueprint* BlueprintBase {nullptr};
	AActor* TemplateActor {nullptr};
};
