#include "Utilities/AGX_HeightFieldUtilities.h"

#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

#include <limits>

namespace
{
	inline bool IsOverlappingPoint(
		int32 X, int32 Y, int32 NumSectionSideX, int32 NumSectionSideY,
		int32 NumVerticesPerSectionSide)
	{
		if (NumSectionSideX == 1 && NumSectionSideY == 1)
			return false;

		if (X > 0 && X % NumVerticesPerSectionSide == 0)
			return true;

		if (Y > 0 && Y % NumVerticesPerSectionSide == 0)
			return true;

		return false;
	}

	inline float ColorToHeight(FColor Color, float ScaleZ)
	{
		constexpr float LANDSCAPE_HEIGHT_SPAN_NOMINAL_M {512.0f};

		// See struct FLandscapeComponentDataInterface::GetHeight for reference.
		uint16 HeightPixel = (Color.R << 8) + Color.G;

		float Frac = (float) HeightPixel / (std::numeric_limits<uint16>::max() + 1);

		// At scale = 100, the height span is 512 meters.
		float Height =
			(Frac - 0.5) * LANDSCAPE_HEIGHT_SPAN_NOMINAL_M * ScaleZ;

		// Height in centimeters.
		return Height;
	}

	struct TextureReadHelper
	{
		// Source: https://isaratech.com/ue4-reading-the-pixels-from-a-utexture2d/
		TextureReadHelper(UTexture2D* InTexture)
		{
			Texture = InTexture;

			// Store original settings
			OldCompressionSettings = Texture->CompressionSettings;
			OldSRGB = Texture->SRGB;
#if WITH_EDITORONLY_DATA
			OldMipGenSettings = Texture->MipGenSettings;
#endif

			// Apply texture settings for reading.
			Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			Texture->SRGB = false;
#if WITH_EDITORONLY_DATA
			Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
			Texture->UpdateResource();
		}

		~TextureReadHelper()
		{
			// Restore texture settings.
			Texture->PlatformData->Mips[0].BulkData.Unlock();
			Texture->CompressionSettings = OldCompressionSettings;
			Texture->SRGB = OldSRGB;
#if WITH_EDITORONLY_DATA
			Texture->MipGenSettings = OldMipGenSettings;
#endif
			Texture->UpdateResource();
		}

		const FColor* GetTextureData() const
		{
			return reinterpret_cast<const FColor*>(
				Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
		}

		UTexture2D* Texture;
		TextureCompressionSettings OldCompressionSettings;
		TextureMipGenSettings OldMipGenSettings;
		bool OldSRGB;
	};
}

FHeightFieldShapeBarrier AGX_HeightFieldUtilities::CreateHeightField(ALandscape& Landscape)
{
	FAGX_LandscapeSizeInfo LandscapeSizeInfo(Landscape);

	if (LandscapeSizeInfo.NumComponents <= 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_HeightFieldUtilities::CreateHeightField cannot create heightfield "
				 "from landscape without components."));

		// Return empty FHeightFieldShapeBarrier (no native allocated).
		return FHeightFieldShapeBarrier();
	}

	TArray<float> Heights;
	Heights.AddUninitialized(LandscapeSizeInfo.NumVertices);

	ULandscapeComponent* Component = Landscape.LandscapeComponents[0];
	UTexture2D* HeightMapTexture = Component->GetHeightmap();

	// Apply necessary settings to the texture to be able to get color data.
	TextureReadHelper TextureReader(HeightMapTexture);
	const FColor* Texturecolor = TextureReader.GetTextureData();
	check(Texturecolor);

	const int32 NumSectionsSideX = LandscapeSizeInfo.NumSectionsSideX;
	const int32 NumSectionsSideY = LandscapeSizeInfo.NumSectionsSideY;
	const int32 NumVerticesPerSectionSide = LandscapeSizeInfo.NumVerticesPerSectionSide;

	// The UTexture2D is always allocated such that it has a sizeX and sizeY that is a power of two
	// and may hold data that goes outside the landscape. The ActualSize below is the data that is
	// actually part of the landscape.
	const int32 ActualSizeX = NumSectionsSideX * NumVerticesPerSectionSide;
	const int32 ActualSizeY = NumSectionsSideY * NumVerticesPerSectionSide;

	// \todo The UTexture2D sizeX and sizeY maxes out at 512 for some reason, meaning we can
	// currently only handle landscapes with vertex count less than that per side.
	if (ActualSizeX > 512 || ActualSizeY > 512)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_HeightFieldUtilities::CreateHeightField landscape has too many quads "
				 "to be able to create a height field from it."));

		// Return empty FHeightFieldShapeBarrier (no native allocated).
		return FHeightFieldShapeBarrier();
	}

	// AGX terrains Y coordinate goes from Unreals Y-max down to zero (flipped).
	int32 Vertex = 0;
	for (int32 Y = ActualSizeY - 1; Y >= 0; Y--)
	{
		for (int32 X = 0; X < ActualSizeX; X++)
		{
			// Unreals landscape counts pixel at section overlap twice.
			if (!IsOverlappingPoint(
					X, Y, NumSectionsSideX, NumSectionsSideY, NumVerticesPerSectionSide))
			{
				FColor PixelColor = Texturecolor[Y * HeightMapTexture->GetSizeX() + X];
				float Height = ColorToHeight(PixelColor, LandscapeSizeInfo.LandscapeScaleZ);
				Heights[Vertex++] = Height;
			}
		}
	}

	check(Vertex == LandscapeSizeInfo.NumVertices);

	const float SideSizeX = LandscapeSizeInfo.NumQuadsSideX * LandscapeSizeInfo.QuadSideSizeX;
	const float SideSizeY = LandscapeSizeInfo.NumQuadsSideY * LandscapeSizeInfo.QuadSideSizeY;

	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(
		LandscapeSizeInfo.NumVerticesSideX, LandscapeSizeInfo.NumVerticesSideY, SideSizeX,
		SideSizeY, Heights);
	return HeightField;
}
