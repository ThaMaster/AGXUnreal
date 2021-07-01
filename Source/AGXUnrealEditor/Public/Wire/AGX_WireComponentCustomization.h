#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Layout/Visibility.h"

class UAGX_WireComponent;

class IDetailLayoutBuilder;


class FAGX_WireComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FText OnGetCurrentSpeed() const;
	FText OnGetCurrentPulledInLength() const;

	EVisibility WithNative() const;
	EVisibility WithoutNative() const;

	TWeakObjectPtr<UAGX_WireComponent> Wire;
};
