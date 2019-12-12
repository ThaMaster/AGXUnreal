#include "AGX_Terrain.h"

// AGXUnreal includes.
#include "AGX_CuttingDirectionComponent.h"
#include "AGX_CuttingEdgeComponent.h"
#include "AGX_LogCategory.h"
#include "AGX_HeightFieldUtilities.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_TopEdgeComponent.h"

// AGXUnrealBarrier includes.
#include "TerrainBarrier.h"
#include "HeightFieldShapeBarrier.h"
#include "Terrain/ShovelBarrier.h"

// Unreal Engine includes.
//#include "NiagaraSystemInstance.h" /// \todo This will be needed once we do particles.
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

AAGX_Terrain::AAGX_Terrain()
{
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

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/// \todo Add vertex offset and particle update here.
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
				TEXT("A shovel in the AGX Terrain '%s' is invalid because the Actor '%s' doesn't have an '%s'"),
				TerrainName, *Owner->GetName(), *TType::StaticClass()->GetName());
			return nullptr;
		}
		return Components[0];
	}
}

void AAGX_Terrain::InitializeNative()
{
	if (SourceLandscape == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("No source landscape selected for terrain %s %s."), *GetActorLabel(), *GetName());
		return;
	}

	if (NativeBarrier.HasNative())
	{
		UE_LOG(LogAGX, Error, TEXT("BeginPlay called on a Terrain that has already been initialized.	"));
		return;
	}

	CreateNativeTerrain();
	CreateNativeShovels();
}

void AAGX_Terrain::CreateNativeTerrain()
{
	FHeightFieldShapeBarrier HeightField = AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	NativeBarrier.AllocateNative(HeightField);
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->AddTerrain(this);
}

void AAGX_Terrain::CreateNativeShovels()
{
	if (!NativeBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeShovels called on Terrain '%s', '%s' which doesn't have a native representation."),
			*GetActorLabel(), *GetName());
	}

	for (FAGX_Shovel& Shovel : Shovels)
	{
		if (Shovel.RigidBodyActor == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("A shovel in the AGX Terrain '%s' is invalid because it does not reference any Actor."),
				*GetName());
			continue;
		}

		AActor* Actor = Shovel.RigidBodyActor;
		UAGX_RigidBodyComponent* Body = GetShovelComponent<decltype(Body)>(Actor, *GetName());
		UAGX_TopEdgeComponent* TopEdge = GetShovelComponent<decltype(TopEdge)>(Actor, *GetName());
		UAGX_CuttingEdgeComponent* CuttingEdge = GetShovelComponent<decltype(CuttingEdge)>(Actor, *GetName());
		UAGX_CuttingDirectionComponent* CuttingDirection =
			GetShovelComponent<decltype(CuttingDirection)>(Actor, *GetName());

		if (Body == nullptr || TopEdge == nullptr || CuttingEdge == nullptr || CuttingDirection == nullptr)
		{
			// GetShovelComponent is responsible for printing the error message.
			continue;
		}

		FTransform WorldToBody = Body->GetComponentTransform().Inverse();

		auto ArrowToLine = [&WorldToBody](UAGX_VectorComponent* Arrow) -> FTwoVectors {
			return {WorldToBody.TransformPosition(Arrow->GetVectorOrigin()),
					WorldToBody.TransformPosition(Arrow->GetVectorTarget())};
		};

		FShovelBarrier ShovelBarrier;
		FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
		FTwoVectors TopEdgeLine = ArrowToLine(TopEdge);
		FTwoVectors CuttingEdgeLine = ArrowToLine(CuttingEdge);
		FVector CuttingDirectionVector = WorldToBody.TransformVector(CuttingDirection->GetVectorDirectionNormalized());
		ShovelBarrier.AllocateNative(*BodyBarrier, TopEdgeLine, CuttingEdgeLine, CuttingDirectionVector);

		bool Added = NativeBarrier.AddShovel(ShovelBarrier);
		if (!Added)
		{
			UE_LOG(LogAGX, Warning, TEXT("Terrain '%s' rejected shovel '%s'."), *GetName(), *Actor->GetName());
			UE_LOG(LogAGX, Warning, TEXT("  Reversing edges"));
			std::swap(TopEdgeLine.v1, TopEdgeLine.v2);
			std::swap(CuttingEdgeLine.v1, CuttingEdgeLine.v2);
			ShovelBarrier.SetTopEdge(TopEdgeLine);
			ShovelBarrier.SetCuttingEdge(CuttingEdgeLine);
			Added = NativeBarrier.AddShovel(ShovelBarrier);
			if (!Added)
			{
				UE_LOG(LogAGX, Error, TEXT("Terrain '%s' rejected shovel '%s'."), *GetName(), *Actor->GetName());
				UE_LOG(LogAGX, Error, TEXT("  Abandoning shovel."));
				continue;
			}
			UE_LOG(
				LogAGX, Warning,
				TEXT("Shovel with reversed edges added successfully. Consider flipping the edges in the editor."));
		}

		UE_LOG(LogAGX, Log, TEXT("Created shovel '%s' for terrain '%s'."), *Actor->GetName(), *GetName());
	}
}
