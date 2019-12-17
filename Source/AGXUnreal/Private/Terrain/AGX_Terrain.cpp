#include "AGX_Terrain.h"

// AGXUnreal includes.
#include "AGX_CuttingDirectionComponent.h"
#include "AGX_CuttingEdgeComponent.h"
#include "AGX_LogCategory.h"
#include "AGX_HeightFieldUtilities.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_TopEdgeComponent.h"
#include "Utilities/AGX_TextureUtilities.h"

// AGXUnrealBarrier includes.
#include "TerrainBarrier.h"
#include "HeightFieldShapeBarrier.h"
#include "Terrain/ShovelBarrier.h"

// Unreal Engine includes.
//#include "NiagaraSystemInstance.h" /// \todo This will be needed once we do particles.
#include "Engine/TextureRenderTarget2D.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"
#include "Misc/AssertionMacros.h"

AAGX_Terrain::AAGX_Terrain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

bool AAGX_Terrain::HasNative()
{
	return NativeBarrier.HasNative();
}

FTerrainBarrier* AAGX_Terrain::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

const FTerrainBarrier* AAGX_Terrain::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

#if WITH_EDITOR
void AAGX_Terrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr)
							 ? PropertyChangedEvent.Property->GetFName()
							 : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, SourceLandscape)))
	{
		if (SourceLandscape == nullptr)
		{
			return;
		}
		int32 NumQuadsSide =
			AGX_HeightFieldUtilities::GetLandscapeSideSizeInQuads(*SourceLandscape);
		float QuadSize = SourceLandscape->GetActorScale().X;
		float Size = QuadSize * NumQuadsSide;
		UE_LOG(
			LogAGX, Display, TEXT("Selected %fcm x %fcm Landscape containing %d x %d quads."),
			Size, Size, NumQuadsSide, NumQuadsSide);
	}
}
#endif

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
}

void AAGX_Terrain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearDisplacementMap();
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateDisplacementMap();

	/// \todo Add particle update here.
}

namespace
{
	template <typename TPtr>
	TPtr GetShovelComponent(AActor* Owner, const TCHAR* TerrainName)
	{
		using TType = typename std::remove_pointer<TPtr>::type;
		TArray<TPtr> Components;
		Owner->GetComponents<TType>(Components);
		if (Components.Num() != 1)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("The shovel '%s' in the AGX Terrain '%s' is invalid because it doesn't have "
					 "exactly one '%s'"),
				*Owner->GetName(), TerrainName, *TType::StaticClass()->GetName());
			return nullptr;
		}
		return Components[0];
	}
}

void AAGX_Terrain::InitializeNative()
{
	if (SourceLandscape == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("No source landscape selected for terrain %s %s."),
			*GetActorLabel(), *GetName());
		return;
	}

	if (NativeBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("BeginPlay called on a Terrain that has already been initialized."));
		return;
	}

	CreateNativeTerrain();
	CreateNativeShovels();
	InitializeRendering();
}

void AAGX_Terrain::CreateNativeTerrain()
{
	FHeightFieldShapeBarrier HeightField =
		AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	NativeBarrier.AllocateNative(HeightField);
	OriginalHeights = NativeBarrier.GetHeights();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->AddTerrain(this);
}

void AAGX_Terrain::CreateNativeShovels()
{
	if (!NativeBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeShovels called on Terrain '%s', '%s' which doesn't have a native "
				 "representation."),
			*GetActorLabel(), *GetName());
	}

	for (FAGX_Shovel& Shovel : Shovels)
	{
		if (Shovel.RigidBodyActor == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("A shovel in the AGX Terrain '%s' is invalid because it does not reference "
					 "any Actor."),
				*GetName());
			continue;
		}

		AActor* Actor = Shovel.RigidBodyActor;
		UAGX_RigidBodyComponent* Body = GetShovelComponent<decltype(Body)>(Actor, *GetName());
		UAGX_TopEdgeComponent* TopEdge = GetShovelComponent<decltype(TopEdge)>(Actor, *GetName());
		UAGX_CuttingEdgeComponent* CuttingEdge =
			GetShovelComponent<decltype(CuttingEdge)>(Actor, *GetName());
		UAGX_CuttingDirectionComponent* CuttingDirection =
			GetShovelComponent<decltype(CuttingDirection)>(Actor, *GetName());

		if (Body == nullptr || TopEdge == nullptr || CuttingEdge == nullptr ||
			CuttingDirection == nullptr)
		{
			// GetShovelComponent is responsible for printing the error message.
			continue;
		}

		FShovelBarrier ShovelBarrier;
		FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
		FTransform const WorldToBody = Body->GetComponentTransform().Inverse();
		FTwoVectors TopEdgeLine = TopEdge->GetInLocal(WorldToBody);
		FTwoVectors CuttingEdgeLine = CuttingEdge->GetInLocal(WorldToBody);
		FVector CuttingDirectionVector =
			WorldToBody.TransformVector(CuttingDirection->GetVectorDirectionNormalized());
		ShovelBarrier.AllocateNative(
			*BodyBarrier, TopEdgeLine, CuttingEdgeLine, CuttingDirectionVector);

		bool Added = NativeBarrier.AddShovel(ShovelBarrier);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Terrain '%s' rejected shovel '%s'. Reversing edge directions and trying "
					 "again."),
				*GetName(), *Actor->GetName());
			std::swap(TopEdgeLine.v1, TopEdgeLine.v2);
			std::swap(CuttingEdgeLine.v1, CuttingEdgeLine.v2);
			ShovelBarrier.SetTopEdge(TopEdgeLine);
			ShovelBarrier.SetCuttingEdge(CuttingEdgeLine);
			Added = NativeBarrier.AddShovel(ShovelBarrier);
			if (!Added)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Terrain '%s' rejected shovel '%s' after edge direction flip. Abandoning "
						 "shovel."),
					*GetName(), *Actor->GetName());
				continue;
			}
			UE_LOG(
				LogAGX, Warning,
				TEXT("Shovel with reversed edges added successfully. Consider flipping the edges "
					 "in the editor."));
		}

		UE_LOG(
			LogAGX, Log, TEXT("Created shovel '%s' for terrain '%s'."), *Actor->GetName(),
			*GetName());
	}
}

void AAGX_Terrain::InitializeRendering()
{
	InitializeDisplacementMap();
	/// \todo Add call to particle system inialization here.
}

void AAGX_Terrain::InitializeDisplacementMap()
{
	if (LandscapeDisplacementMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("No landscape displacement map configured for terrain '%s'. Landscape rendering "
				 "will not include height updates."),
			*GetName());
		return;
	}

	if (LandscapeDisplacementMap->GetFormat() != EPixelFormat::PF_R16F)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The displacement map pixel format for the terrain '%s' must be R16F."),
			*GetName());
		return;
	}

	int32 GridSizeX = NativeBarrier.GetGridSizeX();
	int32 GridSizeY = NativeBarrier.GetGridSizeY();
	if (LandscapeDisplacementMap->SizeX != GridSizeX ||
		LandscapeDisplacementMap->SizeY != GridSizeY)
	{
		UE_LOG(
			LogAGX, Verbose,
			TEXT("The size of the Displacement Map render target (%dx%d) for "
				 "AGX Terrain '%s' does not match the vertices in the terrain (%dx%d). "
				 "Resizing the displacement map."),
			LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY, *GetName(), GridSizeX,
			GridSizeY);

		LandscapeDisplacementMap->ResizeTarget(GridSizeX, GridSizeY);
	}
	if (LandscapeDisplacementMap->SizeX != GridSizeX ||
		LandscapeDisplacementMap->SizeY != GridSizeY)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Landscape displacement map for terrain '%s' could not be resized. Remain at "
				 "(%dx%d). There may be rendering issues."),
			*GetName(), LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY);
	}

	/// \todo I'm not sure why we need this. Does the texture sampler "fudge the
	/// values" when using non-linear gamma?
	LandscapeDisplacementMap->bForceLinearGamma = true;

	DisplacementMapInitialized = true;
}

void AAGX_Terrain::UpdateDisplacementMap()
{
	if (!DisplacementMapInitialized)
	{
		return;
	}
	if (LandscapeDisplacementMap == nullptr)
	{
		return;
	}
	if (!NativeBarrier.HasNative())
	{
		return;
	}

	const int32 NumVerticesX = NativeBarrier.GetGridSizeX();
	const int32 NumVerticesY = NativeBarrier.GetGridSizeY();
	const int32 NumPixels = NumVerticesX * NumVerticesY;
	if (DisplacementData.Num() != NumPixels)
	{
		// This is expected to only happen once. Terrain don't resize during simulation.
		DisplacementData.SetNum(NumPixels);
	}

	if (DisplacementMapRegions.Num() == 0)
	{
		DisplacementMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, NumVerticesX, NumVerticesY));
	}

	TArray<float> CurrentHeights = NativeBarrier.GetHeights();
	for (int32 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
	{
		const float OriginalHeight = OriginalHeights[PixelIndex];
		const float CurrentHeight = CurrentHeights[PixelIndex];
		const float HeightChange = CurrentHeight - OriginalHeight;
		DisplacementData[PixelIndex] = static_cast<FFloat16>(HeightChange);
	}

	uint32 BytesPerPixel = sizeof(FFloat16);
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	AGX_TextureUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 0, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}

void AAGX_Terrain::ClearDisplacementMap()
{
	if (!DisplacementMapInitialized)
	{
		return;
	}
	if (LandscapeDisplacementMap == nullptr)
	{
		return;
	}
	if (!NativeBarrier.HasNative())
	{
		return;
	}
	if (DisplacementMapRegions.Num() == 0)
	{
		return;
	}

	const int32 NumVerticesX = NativeBarrier.GetGridSizeX();
	const uint32 BytesPerPixel = sizeof(FFloat16);
	for (FFloat16& Displacement : DisplacementData)
	{
		Displacement = FFloat16();
	}
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	AGX_TextureUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 0, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}
