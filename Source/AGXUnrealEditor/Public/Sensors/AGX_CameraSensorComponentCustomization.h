// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Camera Sensor Component in the Editor's details panel.
 */
class AGXUNREALEDITOR_API FAGX_CameraSensorComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply OnGenerateRuntimeAssetsButtonClicked();

	IDetailLayoutBuilder* DetailBuilder {nullptr};
};
