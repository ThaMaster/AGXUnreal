// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"

class UTextureRenderTarget2D;
class UMaterial;
struct FUpdateTextureRegion2D;

class AGXUNREAL_API FAGX_TextureUtilities
{
public:
	static bool UpdateRenderTextureRegions(
		UTextureRenderTarget2D& Texture, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
		uint32 SourcePitch, uint32 SourceBitsPerPixel, uint8* SourceData, bool bFreeData);

	/**
	 * Tries to load and returns a UMaterial given an asset path. Returns nullptr if the material
	 * could not be loaded.
	 */
	static UMaterial* GetMaterialFromAssetPath(const TCHAR* AssetPath);
};
