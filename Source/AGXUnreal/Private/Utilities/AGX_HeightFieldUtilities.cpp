#include "Utilities/AGX_HeightFieldUtilities.h"

#include "AGX_LogCategory.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

#include <limits>

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

	inline bool IsOverlappingPoint(
		int32 X, int32 Y, int32 NumSectionSides, int32 NumVerticesPerSectionSide)
	{
		if (NumSectionSides == 1)
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
		constexpr float LANDSCAPE_ZERO_OFFSET_CM {100.0f};

		// See struct FLandscapeComponentDataInterface::GetHeight for reference.
		uint16 HeightPixel = (Color.R << 8) + Color.G;

		float Frac = (float) HeightPixel / (std::numeric_limits<uint16>::max() + 1);

		// At scale = 100, the height span is 512 meters.
		float Height =
			(Frac - 0.5) * LANDSCAPE_HEIGHT_SPAN_NOMINAL_M * ScaleZ + LANDSCAPE_ZERO_OFFSET_CM;

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
			return static_cast<const FColor*>(
				Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
		}

		UTexture2D* Texture;
		TextureCompressionSettings OldCompressionSettings;
		TextureMipGenSettings OldMipGenSettings;
		bool OldSRGB;
	};
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
			LogAGX, Error,
			TEXT("AGX_HeightFieldUtilities::CreateHeightField cannot create heightfield "
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
	const int32 NumSectionSides = NumComponentsSide * Landscape.NumSubsections;
	const int32 NumVerticesPerSectionSide = Landscape.SubsectionSizeQuads + 1;

	TArray<float> Heights;
	Heights.AddUninitialized(NumVertices);

	ULandscapeComponent* Component = Landscape.LandscapeComponents[0];
	UTexture2D* HeightMapTexture = Component->GetHeightmap();

	// Apply necessary settings to the texure to be able to get color data.
	TextureReadHelper TextureReader(HeightMapTexture);
	const FColor* Texturecolor = TextureReader.GetTextureData();
	check(Texturecolor);

	// The UTexture2D is always allocated such that it has a sizeX and sizeY that is a power of two
	// and may hold data that goes outside the landscape. The ActualSize below is the data that is
	// actually part of the landscape.
	const int32 ActualSizeX = NumSectionSides * NumVerticesPerSectionSide;
	const int32 ActualSizeY = NumSectionSides * NumVerticesPerSectionSide;

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
			if (!IsOverlappingPoint(X, Y, NumSectionSides, NumVerticesPerSectionSide))
			{
				FColor PixelColor = Texturecolor[Y * HeightMapTexture->GetSizeX() + X];
				float Height = ColorToHeight(PixelColor, LandscapeScaleZ);
				Heights[Vertex++] = Height;
			}
		}
	}

	check(Vertex == NumVertices);

	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(NumVerticesPerSide, NumVerticesPerSide, SideSize, SideSize, Heights);
	return HeightField;
}
