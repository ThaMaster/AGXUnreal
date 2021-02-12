#pragma once

#include "CoreMinimal.h"

class UTextureRenderTarget2D;
struct FUpdateTextureRegion2D;

namespace AGX_TextureUtilities
{
	bool UpdateRenderTextureRegions(
		UTextureRenderTarget2D& Texture, uint32 NumRegions,
		FUpdateTextureRegion2D* Regions, uint32 SourcePitch, uint32 SourceBitsPerPixel,
		uint8* SourceData, bool bFreeData);
}
