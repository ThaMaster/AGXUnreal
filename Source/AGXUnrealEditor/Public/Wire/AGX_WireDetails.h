// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "IDetailCustomization.h"

class UAGX_WireComponent;

class FAGX_WireDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~ Begin IDetailCustomization interface.
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//~ End IDetailCustomization interface.

public:
	TWeakObjectPtr<UAGX_WireComponent> Wire;
};
