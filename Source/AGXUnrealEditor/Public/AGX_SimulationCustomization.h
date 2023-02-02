// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the RigidBody Component in the Editor.
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
