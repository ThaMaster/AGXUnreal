#include "Utilities/AGX_HeightFieldUtilities.h"

#include "AGX_LogCategory.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

namespace
{
	// A version of the square root that only allows roots that is an integer.
	int32 SqrtInt32(int32 Value)
	{
		const float ValueFloat = static_cast<float>(Value);
		const float RootFloat = FMath::Sqrt(ValueFloat);
		const int32 Root = FMath::RoundToInt(RootFloat);
		check(Root * Root == Value); /// \todo This should be an Error, not a crash.
		return Root;
	}

	bool IsOverlappingPoint(
		int32 X, int32 Y, int32 NumComponentsSide, int32 NumVertsPerComponentSide)
	{
		if (NumComponentsSide == 1)
			return false;

		if (X > 0 && X % NumVertsPerComponentSide == 0)
			return true;

		if (Y > 0 && Y % NumVertsPerComponentSide == 0)
			return true;

		return false;
	}

	float ColorToHeight(FColor Color, float ScaleZ)
	{
		// See struct FLandscapeComponentDataInterface::GetHeight for reference.
		uint16 HeightPixel = (Color.R << 8) + Color.G;

		float Frac = (float) HeightPixel / 65536;

		// At scale = 100, the height span is 512 meters.
		float Height = (Frac - 0.5) * 512 * ScaleZ + 100;

		// Height in centimeters.
		return Height;
	}
}

int32 AGX_HeightFieldUtilities::GetLandscapeSideSizeInQuads(ALandscape& Landscape)
{
	// This assumes a square and uniform grid of components.
	/// \todo Figure out how to get the size/sides of the component grid.
	const int32 NumComponents = Landscape.LandscapeComponents.Num();
	const int32 NumComponentsSide = SqrtInt32(NumComponents);
	const int32 NumQuadsPerComponentSide = Landscape.ComponentSizeQuads;
	const int32 NumQuadsPerSide = NumComponentsSide * NumQuadsPerComponentSide;
	return NumQuadsPerSide;
}

FHeightFieldShapeBarrier AGX_HeightFieldUtilities::CreateHeightField(ALandscape& Landscape)
{
	const int32 NumComponents = Landscape.LandscapeComponents.Num();

	if (NumComponents <= 0)
	{
		UE_LOG(
			LogAGX, Error, TEXT("AGX_HeightFieldUtilities::CreateHeightField cannot create heightfield "
			"from landscape without components."));

		// Return empty FHeightFieldShapeBarrier (no native allocated).
		return FHeightFieldShapeBarrier();
	}

	// This assumes a square and uniform grid of components.
	/// \todo Figure out how to get the size/sides of the component grid.
	const int32 NumComponentsSide =
		FMath::RoundToInt(FMath::Sqrt(static_cast<float>(NumComponents)));
	check(NumComponentsSide * NumComponentsSide == NumComponents);

	const int32 NumQuadsPerComponentSide = Landscape.ComponentSizeQuads;
	const int32 NumQuadsPerSide = NumComponentsSide * NumQuadsPerComponentSide;
	const int32 NumVerticesPerSide = NumQuadsPerSide + 1;
	const int32 NumVertices = NumVerticesPerSide * NumVerticesPerSide;
	const float QuadSideSize = Landscape.GetActorScale().X;
	const float SideSize = NumQuadsPerSide * QuadSideSize;
	const float LandscapeScaleZ = Landscape.GetActorScale3D().Z;

	TArray<float> Heights;
	Heights.AddUninitialized(NumVertices);

	ULandscapeComponent* Component = Landscape.LandscapeComponents[0];
	UTexture2D* HeightMapTexture = Component->GetHeightmap();

	// Apply necessary settings to the texure to be able to get color data.
	// Source: https://isaratech.com/ue4-reading-the-pixels-from-a-utexture2d/
	TextureCompressionSettings OldCompressionSettings = HeightMapTexture->CompressionSettings;
	HeightMapTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
#if WITH_EDITORONLY_DATA
	TextureMipGenSettings OldMipGenSettings = HeightMapTexture->MipGenSettings;
	HeightMapTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	bool OldSRGB = HeightMapTexture->SRGB;
	HeightMapTexture->SRGB = false;
	HeightMapTexture->UpdateResource();

	const FColor* Texturecolor =
		static_cast<const FColor*>(HeightMapTexture->PlatformData->Mips[0].BulkData.LockReadOnly());

	// AGX terrains Y coordinate goes from Unreals Y-max down to zero (flipped).
	int32 Vertex = 0;
	for (int32 Y = HeightMapTexture->GetSizeY() - 1; Y >= 0; Y--)
	{
		for (int32 X = 0; X < HeightMapTexture->GetSizeX(); X++)
		{
			// Unreals landscape counts pixel at component overlap twice.
			if (!IsOverlappingPoint(X, Y, NumComponentsSide, NumQuadsPerComponentSide + 1))
			{
				FColor PixelColor = Texturecolor[Y * HeightMapTexture->GetSizeX() + X];
				float Height = ColorToHeight(PixelColor, LandscapeScaleZ);

				Heights[Vertex++] = Height;
			}
		}
	}

	check(Vertex == NumVertices);

	// Restore texture settings.
	HeightMapTexture->PlatformData->Mips[0].BulkData.Unlock();
	HeightMapTexture->CompressionSettings = OldCompressionSettings;
#if WITH_EDITORONLY_DATA
	HeightMapTexture->MipGenSettings = OldMipGenSettings;
#endif
	HeightMapTexture->SRGB = OldSRGB;
	HeightMapTexture->UpdateResource();

	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(NumVerticesPerSide, NumVerticesPerSide, SideSize, SideSize, Heights);
	return HeightField;
}
