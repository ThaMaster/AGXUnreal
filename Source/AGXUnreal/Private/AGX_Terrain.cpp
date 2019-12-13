// AGXUnreal includes.
#include "AGX_Terrain.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "AGX_HeightFieldUtilities.h"

// AGXUnrealBarrier includes.
#include "TerrainBarrier.h"
#include "HeightFieldShapeBarrier.h"

// Unreal Engine includes.
//#include "NiagaraSystemInstance.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

AAGX_Terrain::AAGX_Terrain()
{
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

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();

	if (SourceLandscape == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("No source landscape selected for terrain %s %s."),
			*GetActorLabel(), *GetName());
		return;
	}

	if (NativeBarrier.HasNative())
	{
		return;
	}

	FHeightFieldShapeBarrier HeightField =
		AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	NativeBarrier.AllocateNative(HeightField);
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->AddTerrain(this);
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/// \todo Add vertex offset and particle update here.
}
