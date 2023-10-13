// Copyright 2023, Algoryx Simulation AB.

#pragma once

class UAGX_ShovelComponent;

class AGXUNREAL_API FAGX_ShovelUtilities
{
public:

	static UAGX_ShovelComponent* GetShovelToModify(UAGX_ShovelComponent* Shovel);

	/**
	 * See FPropertyHandleFloat::SetValue in PropertyHandleImpl.cpp.
	 * See Expose_TFormatSpecifier(double in UnrealTypeTraits.h.
	 */
	static void TruncateForDetailsPanel(double& Value);
	static void TruncateForDetailsPanel(FVector& Values);

	// The size of Hit Proxies created by the various Shovel-related Visualizers we have.
	// This may become a configurable setting in the future.
	static constexpr float HitProxySize = 10.0f; // todo Where should this value live?
};
