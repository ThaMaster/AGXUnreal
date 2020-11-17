#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_SimulationObjectComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

class AGXUNREALEDITOR_API FAGX_SimulationObjectCustomization : public IDetailCustomization
{
public:
	static TSharedRef <IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
