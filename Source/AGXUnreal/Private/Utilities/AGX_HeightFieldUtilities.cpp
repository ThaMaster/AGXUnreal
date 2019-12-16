#include "AGX_HeightFieldUtilities.h"

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

	TArray<float> Heights;
	Heights.AddUninitialized(NumVertices);

	auto WriteComponent = [&Heights, NumVerticesPerSide](ULandscapeComponent& Component) {
		const int32 BaseQuadX = Component.SectionBaseX;
		const int32 BaseQuadY = Component.SectionBaseY;

		const int32 NumQuadsPerComponentSide = Component.ComponentSizeQuads;
		const int32 NumVerticesPerComponentSide = NumQuadsPerComponentSide + 1;

		FLandscapeComponentDataInterface ComponentData(&Component);

		auto GlobalAGXFromLocalUnreal = [NumVerticesPerSide, BaseQuadX, BaseQuadY](
											int32 LocalVertexX, int32 LocalVertexY) {
			const int32 GlobalVertexX = BaseQuadX + LocalVertexX;
			const int32 GlobalVertexY = BaseQuadY + LocalVertexY;
			const int32 AGXGlobalVertexX = GlobalVertexX;
			const int32 AGXGlobalVertexY = (NumVerticesPerSide - 1) - GlobalVertexY;
			return (NumVerticesPerSide * AGXGlobalVertexY) + AGXGlobalVertexX;
		};

		for (int32 Y = 0; Y < NumVerticesPerComponentSide; ++Y)
		{
			const int32 BaseIndex = GlobalAGXFromLocalUnreal(0, Y);
			for (int32 X = 0; X < NumVerticesPerComponentSide; ++X)
			{
				const float Height = ComponentData.GetWorldVertex(X, Y).Z;
				Heights[BaseIndex + X] = Height;
			}
		}
	};

	for (ULandscapeComponent* Component : Landscape.LandscapeComponents)
	{
		WriteComponent(*Component);
	}

	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(NumVerticesPerSide, NumVerticesPerSide, SideSize, SideSize, Heights);
	return HeightField;
}
