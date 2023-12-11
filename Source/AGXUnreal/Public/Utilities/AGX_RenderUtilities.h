// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

class FShapeContactBarrier;
class UTextureRenderTarget2D;
class UMaterial;
struct FUpdateTextureRegion2D;

class AGXUNREAL_API FAGX_RenderUtilities
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

	/**
	 * Renders the given ShapeContacts to the screen.
	 * The rendering is not avaiable in built applications built with Shipping configuration.
	 */
	static void DrawContactPoints(
		const TArray<FShapeContactBarrier>& ShapeContacts, float LifeTime, UWorld* World);
};
