// Author: VMC Motion Technologies Co., Ltd.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackRenderer.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Layout/Visibility.h"

class UAGX_TrackRenderer;

class IDetailLayoutBuilder;

/**
 *
 */
class AGXUNREALEDITOR_API FAGX_TrackRendererDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
};
