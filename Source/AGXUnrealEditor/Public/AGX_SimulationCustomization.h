// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Simulation (config) Component in the Editor.
 * This is what is seen in Project Settings > AGX Dynamics.
 */
class AGXUNREALEDITOR_API FAGX_SimulationCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

private:
	FText GetOutputFilePathText() const;
	FReply OnBrowseFileButtonClicked();

	IDetailLayoutBuilder* DetailBuilder;
};
