#include "Terrain/AGX_Terrain.h"

// AGXUnreal includes.
#include "Terrain/AGX_CuttingDirectionComponent.h"
#include "Terrain/AGX_CuttingEdgeComponent.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Terrain/AGX_TopEdgeComponent.h"
#include "Utilities/AGX_TextureUtilities.h"
#include "Materials/AGX_TerrainMaterialInstance.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Materials/AGX_MaterialBase.h"

// AGXUnrealBarrier includes.
#include "Terrain/TerrainBarrier.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Terrain/ShovelBarrier.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"
#include "Misc/AssertionMacros.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AAGX_Terrain::AAGX_Terrain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	// Create a root SceneComponent so that this Actor has a transform
	// which can be modified in the Editor.
	{
		USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());

		Root->Mobility = EComponentMobility::Static;
		Root->SetFlags(Root->GetFlags() | RF_Transactional); /// \todo What does this mean?

#if WITH_EDITORONLY_DATA
		Root->bVisualizeComponent = true;
#endif

		SetRootComponent(Root);
	}
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
			LogAGX, Display, TEXT("Selected %fcm x %fcm Landscape containing %d x %d quads."), Size,
			Size, NumQuadsSide, NumQuadsSide);
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
	ClearParticlesMap();
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateDisplacementMap();
	UpdateParticlesMap();
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
	CreateTerrainMaterial();
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

		FAGX_Shovel::UpdateNativeShovelProperties(ShovelBarrier, Shovel);

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
	InitializeParticleSystem();
	InitializeParticlesMap();
}

void AAGX_Terrain::CreateTerrainMaterial()
{
	if (!HasNative())
		return;

	if (TerrainMaterial)
	{
		// Both an UAGX_TerrainMaterialInstance and a UAGX_ShapeMaterialInstance
		// are set for the terrain. The former is the native agxTerrain::TerrainMaterial
		// counterpart and the latter is the native agx::Material counterpart.

		// Set TerrainMaterial
		UAGX_TerrainMaterialInstance* TerrainMaterialInstance =
			static_cast<UAGX_TerrainMaterialInstance*>(
				TerrainMaterial->GetOrCreateInstance(GetWorld()));

		check(TerrainMaterialInstance);

		FTerrainMaterialBarrier* TerrainMaterialBarrier =
			TerrainMaterialInstance->GetOrCreateTerrainMaterialNative(GetWorld());
		check(TerrainMaterialBarrier);

		UE_LOG(
			LogAGX, Log,
			TEXT(
				"AAGX_Terrain::CreateTerrainMaterial is setting native material %s on terrain %s."),
			*TerrainMaterialInstance->GetName(), *GetName());

		GetNative()->SetTerrainMaterial(*TerrainMaterialBarrier);

		// Set ShapeMaterial
		FShapeMaterialBarrier* MaterialBarrier =
			TerrainMaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
		check(MaterialBarrier);

		GetNative()->SetShapeMaterial(*MaterialBarrier);

		// Swap properties
		UWorld* PlayingWorld = GetWorld();

		if (TerrainMaterialInstance != TerrainMaterial && PlayingWorld &&
			PlayingWorld->IsGameWorld())
		{
			UE_LOG(
				LogAGX, Log,
				TEXT("AAGX_Terrain::CreateTerrainMaterial is swapping a property "
					 "(to %s from %s)."),
				*GetNameSafe(TerrainMaterialInstance), *GetNameSafe(TerrainMaterial));

			TerrainMaterial = TerrainMaterialInstance;
		}
	}
}

void AAGX_Terrain::InitializeDisplacementMap()
{
	if (LandscapeDisplacementMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("No landscape displacement map configured for terrain '%s'. Terrain rendering "
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

	// "Grid" in the Terrain is what the Landscape calls "vertices". There is
	// one more "grid" element than there is "quads" per side. There is one
	// displacement map texel per vertex.
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
			TEXT("Landscape displacement map for terrain '%s' could not be resized. "
				 "There may be rendering issues."),
			*GetName(), LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY);
	}

	DisplacementData.SetNum(GridSizeX * GridSizeY);
	DisplacementMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, GridSizeX, GridSizeY));

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
	check(DisplacementData.Num() == NumPixels);
	check(DisplacementMapRegions.Num() == 1);

	TArray<float> CurrentHeights = NativeBarrier.GetHeights();
	for (int32 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
	{
		const float HeightChange = CurrentHeights[PixelIndex] - OriginalHeights[PixelIndex];
		DisplacementData[PixelIndex] = static_cast<FFloat16>(HeightChange);
	}

	uint32 BytesPerPixel = sizeof(FFloat16);
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	AGX_TextureUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
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
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}

namespace
{
	/**
	Calculates and returns the smallest base size of a square sized texture,
	such that the base size is evenly divisible by pixelsPerItem and has a square
	that is at least minNumItems x pixelsPerItem.
	*/
	int32 CalculateTextureBaseSize(int32 minNumItems, int32 PixelsPerItem)
	{
		const int32 maxSize = 8192;
		for (int32 base = FMath::Sqrt(minNumItems * PixelsPerItem); base <= maxSize; ++base)
		{
			if ((base % PixelsPerItem == 0) && (base * base >= minNumItems * PixelsPerItem))
				return base;
		}
		check(!"CalculateTextureBaseSize failed");
		return 0;
	}
}

void AAGX_Terrain::InitializeParticleSystem()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' does not have a particle system, cannot render particles"),
			*GetName());
		return;
	}

	ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
		FVector::OneVector, EAttachLocation::Type::KeepWorldPosition, false, EPSCPoolMethod::None);
#if WITH_EDITORONLY_DATA
	ParticleSystemComponent->bVisualizeComponent = true;
#endif
}

void AAGX_Terrain::InitializeParticlesMap()
{
	if (TerrainParticlesDataMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("No particles data map configured for terrain '%s'. Terrain rendering will not "
				 "include particle."),
			*GetName());
		return;
	}

	if (TerrainParticlesDataMap->GetFormat() != EPixelFormat::PF_A32B32G32R32F)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The particle data map pixel format for the terrain '%s' must be RGBA32F."),
			*GetName());
		return;
	}

	// Finds the closest fitting base size of a square sized texture such that it has room for all
	// particles data and such that the base size is a multiple of number of pixels per particle
	// (because we do not want a row-break in the middle of a particle element, since that increases
	// complexity of the Niagara Module Script).
	const int32 TextureBaseSize =
		CalculateTextureBaseSize(MaxNumRenderParticles, NumPixelsPerParticle);
	check(TextureBaseSize % NumPixelsPerParticle == 0);
	check(TextureBaseSize * TextureBaseSize >= MaxNumRenderParticles * NumPixelsPerParticle);

	if (TerrainParticlesDataMap->SizeX != TextureBaseSize ||
		TerrainParticlesDataMap->SizeY != TextureBaseSize)
	{
		UE_LOG(
			LogAGX, Verbose,
			TEXT("The size of the particle data render target (%dx%d) for "
				 "AGX Terrain '%s' does not match the amount data required to hold the terrain "
				 "particle data. Resizing the displacement map."),
			TerrainParticlesDataMap->SizeX, TerrainParticlesDataMap->SizeY, *GetName());

		TerrainParticlesDataMap->ResizeTarget(TextureBaseSize, TextureBaseSize);
	}

	ParticleSystemInitialized = true;
}

void AAGX_Terrain::UpdateParticlesMap()
{
	if (!ParticleSystemInitialized)
	{
		return;
	}

	const int32 ResolutionX = TerrainParticlesDataMap->SizeX;
	const int32 ResolutionY = TerrainParticlesDataMap->SizeY;
	const int32 NumPixels = ResolutionX * ResolutionY;
	const int32 NumComponentsPerPixel = 4;
	const int32 NumBytesPerPixel = NumComponentsPerPixel * sizeof(FFloat32);
	const int32 NumComponentsPerParticle = NumComponentsPerPixel * NumPixelsPerParticle;
	const int32 NumBytes = NumPixels * NumBytesPerPixel;
	const int32 MaxNumParticles = NumPixels / NumPixelsPerParticle;

	if (TerrainParticlesData.Num() == 0)
	{
		TerrainParticlesData.SetNum(NumComponentsPerPixel * NumPixels);
	}

	if (ParticlesDataMapRegions.Num() == 0)
	{
		ParticlesDataMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, ResolutionX, ResolutionY));
	}

	TArray<FVector> Positions = NativeBarrier.GetParticlePositions();
	TArray<float> Radii = NativeBarrier.GetParticleRadii();
	TArray<FQuat> Rotations = NativeBarrier.GetParticleRotations();

	check(Positions.Num() == Radii.Num());
	check(Positions.Num() == Rotations.Num());

	int32 NumParticles = FMath::Min(Positions.Num(), MaxNumParticles);

	ParticleSystemComponent->SetNiagaraVariableInt("User.TargetParticleCount", NumParticles);
	for (int32 ParticleIndex = 0, PixelIndex = 0; ParticleIndex < NumParticles;
		 ++ParticleIndex, PixelIndex += NumComponentsPerParticle)
	{
		// Multiply position by 0.01 because it seems we need to pack floats to
		// smaller range. The position floats are unpacked in the
		// `GetTerrainParticleData` Niagara Module Script.
		/// \todo Investigate!
		const float PackingScale = 0.01f;
		TerrainParticlesData[PixelIndex + 0] = Positions[ParticleIndex].X * PackingScale;
		TerrainParticlesData[PixelIndex + 1] = Positions[ParticleIndex].Y * PackingScale;
		TerrainParticlesData[PixelIndex + 2] = Positions[ParticleIndex].Z * PackingScale;

		// The particle size slot in the render target is a scale, not the
		// actual size. The scale is relative to a SI unit cube, meaning that a
		// scale of 1.0 should render a particle that is 1x1x1 m large, or
		// 100x100x100 Unreal units. We multiply by 2.0 to convert from radius
		// to full width.
		float UnitCubeScale = (Radii[ParticleIndex] * 2.0f) / 100.0f;
		TerrainParticlesData[PixelIndex + 3] = UnitCubeScale;

		TerrainParticlesData[PixelIndex + 4] = Rotations[ParticleIndex].X;
		TerrainParticlesData[PixelIndex + 5] = Rotations[ParticleIndex].Y;
		TerrainParticlesData[PixelIndex + 6] = Rotations[ParticleIndex].Z;
		TerrainParticlesData[PixelIndex + 7] = Rotations[ParticleIndex].W;
	}

	uint8* PixelData = reinterpret_cast<uint8*>(TerrainParticlesData.GetData());
	AGX_TextureUtilities::UpdateRenderTextureRegions(
		*TerrainParticlesDataMap, 1, ParticlesDataMapRegions.GetData(),
		ResolutionX * NumBytesPerPixel, NumBytesPerPixel, PixelData, false);
}

void AAGX_Terrain::ClearParticlesMap()
{
	if (!ParticleSystemInitialized)
	{
		return;
	}
	if (TerrainParticlesDataMap == nullptr)
	{
		return;
	}
	if (!NativeBarrier.HasNative())
	{
		return;
	}
	if (ParticlesDataMapRegions.Num() == 0)
	{
		return;
	}

	const int32 ResolutionX = TerrainParticlesDataMap->SizeX;
	const int32 NumComponentsPerPixel = 4;
	const int32 NumBytesPerPixel = NumComponentsPerPixel * sizeof(FFloat32);
	for (FFloat32& Pixel : TerrainParticlesData)
	{
		Pixel = FFloat32();
	}
	uint8* PixelData = reinterpret_cast<uint8*>(TerrainParticlesData.GetData());
	AGX_TextureUtilities::UpdateRenderTextureRegions(
		*TerrainParticlesDataMap, 1, ParticlesDataMapRegions.GetData(),
		ResolutionX * NumBytesPerPixel, NumBytesPerPixel, PixelData, false);
}
