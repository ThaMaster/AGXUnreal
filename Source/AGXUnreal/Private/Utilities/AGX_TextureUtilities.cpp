#include "Utilities/AGX_TextureUtilities.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/TextureRenderTarget2D.h"
#include "RHI.h"
#include "RHICommandList.h"

bool AGX_TextureUtilities::UpdateRenderTextureRegions(
	UTextureRenderTarget2D& RenderTarget, int32 MipIndex, uint32 NumRegions,
	FUpdateTextureRegion2D* Regions, uint32 SourcePitch, uint32 SourceBitsPerPixel,
	uint8* SourceData, bool bFreeData)
{
	if (RenderTarget.Resource == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("TextureRenderTarget doesn't have a resource."));
		return false;
	}

	/// \todo Get rid of this pixel, I don't think we need it. Extend lambda
	/// capture list instead.
	struct FUpdateTextureRegionsDescription
	{
		FTextureRenderTarget2DResource* Texture2DResource;
		int32 MipIndex;
		uint32 NumRegions;
		FUpdateTextureRegion2D* Regions;
		uint32 SourcePitch;
		uint32 SourceBitsPerPixel;
		uint8* SourceData;
	};

	/// \todo Why can't Cast<> be used here?
	FUpdateTextureRegionsDescription* RegionsDescription = new FUpdateTextureRegionsDescription;
	RegionsDescription->Texture2DResource = (FTextureRenderTarget2DResource*)(RenderTarget.Resource);
	RegionsDescription->MipIndex = MipIndex;
	RegionsDescription->NumRegions = NumRegions;
	RegionsDescription->Regions = Regions;
	RegionsDescription->SourcePitch = SourcePitch;
	RegionsDescription->SourceBitsPerPixel = SourceBitsPerPixel;
	RegionsDescription->SourceData = SourceData;

	ENQUEUE_RENDER_COMMAND(UpdateRenderTextureRegionsData)
	([RegionsDescription, bFreeData](FRHICommandListImmediate& RHICmdList) {
		FRHITexture2D* Texture = RegionsDescription->Texture2DResource->GetTextureRHI();
		uint32 Pitch = RegionsDescription->SourcePitch;
		uint32 BitsPerPixel = RegionsDescription->SourceBitsPerPixel;
		uint8* SourceData = RegionsDescription->SourceData;
		for (uint32 RegionIndex = 0; RegionIndex < RegionsDescription->NumRegions; ++RegionIndex)
		{
			FUpdateTextureRegion2D& Region = RegionsDescription->Regions[RegionIndex];
			uint8* Bits = SourceData + Region.SrcY * Pitch + Region.SrcX * BitsPerPixel;
			RHIUpdateTexture2D(Texture, /*MipIndex*/ 0, Region, Pitch, Bits);
		}
		if (bFreeData)
		{
			FMemory::Free(RegionsDescription->Regions);
			FMemory::Free(RegionsDescription->SourceData);
		}
		delete RegionsDescription;
	});

	return true;
}
