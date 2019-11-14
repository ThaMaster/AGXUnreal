// Fill out your copyright notice in the Description page of Project Settings.

#include "AGX_Terrain.h"
#include "AGX_LogCategory.h"

#include "HeightFieldShapeBarrier.h"

//#include "NiagaraSystemInstance.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

AAGX_Terrain::AAGX_Terrain()
{
}

#define MY_LOG(V, t) UE_LOG(LogTemp, Log, TEXT(#V ": %" #t), V)

/// \todo Consider moving to a LandscapeUtilities, or similar, file.
namespace
{
	void WriteComponent(ULandscapeComponent& Component, TArray<float> Heights, int32 NumVerticesPerLandscapeSide)
	{
		/*
		Example when using:
		- 3x3 components per landscape.
		- 2x2 sections per component.
		- 7x7 quads per section.

		NumComponents: 9
		NumComponentsSide: 3
		NumQuadsPerComponentSide: 14
		NumQuadsPerSide: 42
		NumVerticesPerSide: 43

		Base: (0, 0)
		Position world0: (-2100.000000, -2100.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (-2000.000000, -2100.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (14, 0)
		Position world0: (-700.000000, -2100.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (-600.000000, -2100.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (28, 0)
		Position world0: (700.000000, -2100.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (800.000000, -2100.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (0, 14)
		Position world0: (-2100.000000, -700.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (-2000.000000, -700.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (14, 14)
		Position world0: (-700.000000, -700.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (-600.000000, -700.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (28, 14)
		Position world0: (700.000000, -700.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (800.000000, -700.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (0, 28)
		Position world0: (-2100.000000, 700.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (-2000.000000, 700.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (14, 28)
		Position world0: (-700.000000, 700.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (-600.000000, 700.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)
		Base: (28, 28)
		Position world0: (700.000000, 700.000000, 100.000000), Position local: (0.000000, 0.000000, 0.000000)
		Position world1: (800.000000, 700.000000, 100.000000), Position local: (1.000000, 0.000000, 0.000000)


		*/

		//		MY_LOG(Component.SectionBaseX, d);
		//		MY_LOG(Component.SectionBaseY, d);

		const int32 NumQuadsPerComponentSide = Component.ComponentSizeQuads;
		const int32 NumVerticesPerSide = NumQuadsPerComponentSide + 1;

		const int32 BaseQuadX = Component.SectionBaseX;
		const int32 BaseQuadY = Component.SectionBaseY;

		FLandscapeComponentDataInterface ComponentData(&Component);

		FVector WorldPoint0 = ComponentData.GetWorldVertex(0, 0);
		FVector LocalPoint0 = ComponentData.GetLocalVertex(0, 0);	// Relative to component?

		FVector WorldPoint1 = ComponentData.GetWorldVertex(1, 0);
		FVector LocalPoint1 = ComponentData.GetLocalVertex(1, 0);	// Relative to component?

		UE_LOG(LogTemp, Log, TEXT("Base: (%d, %d)"), BaseQuadX, BaseQuadY);

		UE_LOG(LogTemp, Log, TEXT("Position world0: (%f, %f, %f), Position local: (%f, %f, %f)"), WorldPoint0.X,
			WorldPoint0.Y, WorldPoint0.Z, LocalPoint0.X, LocalPoint0.Y, LocalPoint0.Z);
		UE_LOG(LogTemp, Log, TEXT("Position world1: (%f, %f, %f), Position local: (%f, %f, %f)"), WorldPoint1.X,
			WorldPoint1.Y, WorldPoint1.Z, LocalPoint1.X, LocalPoint1.Y, LocalPoint1.Z);
		auto GlobalAGXFromLocalUnreal = [NumVerticesPerLandscapeSide, BaseQuadX, BaseQuadY](
											int32 LocalVertexX, int32 LocalVertexY)
		{
			const int32 GlobalVertexX = BaseQuadX + LocalVertexX;
			const int32 GlobalVertexY = BaseQuadY + LocalVertexY;
			const int32 AGXGlobalVertexX = GlobalVertexX;
			const int32 AGXGlobalVertexY = (NumVerticesPerLandscapeSide - 1) - GlobalVertexY;
			return (NumVerticesPerLandscapeSide * AGXGlobalVertexY) + AGXGlobalVertexX;
		};

		auto WriteComponentRow = [&Heights, &GlobalAGXFromLocalUnreal, &ComponentData, NumVerticesPerSide](const int32 Y)
		{
			const int32 BaseIndex = GlobalAGXFromLocalUnreal(0, Y);
			for (int32 X = 0; X < NumVerticesPerSide; ++X)
			{
				const float Height = ComponentData.GetWorldVertex(X, Y).Z;
				UE_LOG(LogTemp, Log, TEXT("Writing height index %d from (%d, %d)."), BaseIndex+X, X, Y);
				Heights[BaseIndex + X] = Height;
			}
		};

		for (int32 Y = 0; Y < NumVerticesPerSide; ++Y)
		{
			// This implementation will write shared verices twice, once at the
			// end of one component row and once again at the beginning of the
			// neighboring component row.
			WriteComponentRow(Y);
		}
	}

	FHeightFieldShapeBarrier CreateHeightField(ALandscape& Landscape)
	{
		FHeightFieldShapeBarrier HeightField;

		const int32 NumComponents = Landscape.LandscapeComponents.Num();

		// This assumes a square and uniform grid of components.
		/// \todo Figure out how to get the size/sides of the component grid.
		const int32 NumComponentsSide = FMath::RoundToInt(FMath::Sqrt(static_cast<float>(NumComponents)));
		check(NumComponentsSide * NumComponentsSide == NumComponents);

		const int32 NumQuadsPerComponentSide = Landscape.ComponentSizeQuads;
		const int32 NumQuadsPerSide = NumComponentsSide * NumQuadsPerComponentSide;

		const int32 NumVerticesPerSide = NumQuadsPerSide + 1;
		const int32 NumVertices = NumVerticesPerSide * NumVerticesPerSide;

		const float QuadSideSize = Landscape.GetActorScale().X;
		const float LandscapeSideSize = NumQuadsPerSide * QuadSideSize;

		// Example when using:
		// - 3x3 components per landscape.
		// - 2x2 sections per component.
		// - 7x7 quads per section.
		MY_LOG(NumComponents, d);				// 9
		MY_LOG(NumComponentsSide, d);			// 3
		MY_LOG(NumQuadsPerComponentSide, d);	// 14
		MY_LOG(NumQuadsPerSide, d);				// 42
		MY_LOG(NumVerticesPerSide, d);			// 43

		TArray<float> Heights;
		Heights.AddUninitialized(NumVertices);

		for (ULandscapeComponent* Component : Landscape.LandscapeComponents)
		{
			WriteComponent(*Component, Heights, NumVerticesPerSide);
		}

		HeightField.AllocateNative(
			NumVerticesPerSide, NumVerticesPerSide, LandscapeSideSize, LandscapeSideSize, Heights);

		return HeightField;
	}
}

#undef MY_LOG

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();

	if (SourceLandscape == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("No source landscape selected for terrain %s %s."), *GetActorLabel(), *GetName());
		return;
	}

	FHeightFieldShapeBarrier HeightField = CreateHeightField(*SourceLandscape);
	NativeBarrier.AllocateNative(HeightField);
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
