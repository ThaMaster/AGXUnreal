// Copyright 2023, Algoryx Simulation AB.

#include "Utilities/AGX_RenderUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Contacts/ShapeContactBarrier.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Misc/EngineVersionComparison.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "TextureResource.h"

bool FAGX_RenderUtilities::UpdateRenderTextureRegions(
	UTextureRenderTarget2D& RenderTarget, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
	uint32 SourcePitch, uint32 SourceBitsPerPixel, uint8* SourceData, bool bFreeData)
{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FTextureRenderTarget2DResource* Resource =
		(FTextureRenderTarget2DResource*) (RenderTarget.Resource);
#else
	FTextureRenderTarget2DResource* Resource =
		(FTextureRenderTarget2DResource*) (RenderTarget.GetResource());
#endif
	if (Resource == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("TextureRenderTarget doesn't have a resource."));
		return false;
	}

	auto WriteTexture = [Resource, NumRegions, Regions, SourcePitch, SourceBitsPerPixel, SourceData,
						 bFreeData](FRHICommandListImmediate& RHICmdList)
	{
		FRHITexture2D* Texture = Resource->GetTextureRHI();
		for (uint32 RegionIndex = 0; RegionIndex < NumRegions; ++RegionIndex)
		{
			FUpdateTextureRegion2D& Region = Regions[RegionIndex];
			uint8* Bits = SourceData + Region.SrcY * SourcePitch + Region.SrcX * SourceBitsPerPixel;
			RHIUpdateTexture2D(Texture, /*MipIndex*/ 0, Region, SourcePitch, Bits);
		}
		if (bFreeData)
		{
			FMemory::Free(Regions);
			FMemory::Free(SourceData);
		}
	};

	ENQUEUE_RENDER_COMMAND(UpdateRenderTextureRegionsData)(std::move(WriteTexture));

	return true;
}

UMaterial* FAGX_RenderUtilities::GetMaterialFromAssetPath(const TCHAR* AssetPath)
{
	UObject* LoadResult = StaticLoadObject(UMaterial::StaticClass(), nullptr, AssetPath);
	if (LoadResult == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_TextureUtilities::GetMaterialFromAssetPath: Unable to load material '%s'."),
			AssetPath);
		return nullptr;
	}

	return Cast<UMaterial>(LoadResult);
}

void FAGX_RenderUtilities::DrawContactPoints(
	const TArray<FShapeContactBarrier>& ShapeContacts, float LifeTime, UWorld* World)
{
	for (const FShapeContactBarrier& ShapeContact : ShapeContacts)
	{
		for (const FContactPointBarrier& ContactPoint : ShapeContact.GetContactPoints())
		{
			const FVector PointLocation = ContactPoint.GetLocation();
			DrawDebugPoint(World, PointLocation, 7.f, FColor::Orange, false, LifeTime, 99);
			
			// The Normal is drawn as a 2 cm line.
			const FVector NormalEnd = PointLocation + ContactPoint.GetNormal() * 2.0;
			DrawDebugLine(World, PointLocation, NormalEnd, FColor::Orange, false, LifeTime, 99, 0.2f);
		}
	}
}
