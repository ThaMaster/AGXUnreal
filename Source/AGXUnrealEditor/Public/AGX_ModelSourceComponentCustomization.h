// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Model Source Component in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_ModelSourceComponentCustomization
	: public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

private:
	FReply OnSynchronizeModelButtonClicked();

	IDetailLayoutBuilder* DetailBuilder;
};
