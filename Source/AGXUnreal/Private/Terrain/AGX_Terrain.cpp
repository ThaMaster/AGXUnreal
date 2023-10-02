// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_Terrain.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_CustomVersion.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "AGX_RigidBodyComponent.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Terrain/AGX_CuttingDirectionComponent.h"
#include "Terrain/AGX_CuttingEdgeComponent.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_TerrainSpriteComponent.h"
#include "Terrain/AGX_TopEdgeComponent.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_TextureUtilities.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"
#include "LandscapeStreamingProxy.h"
#include "Misc/AssertionMacros.h"
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectIterator.h"

// Standard library includes.
#include <algorithm>

#ifdef LOCTEXT_NAMESPACE
#error "LOCTEXT_NAMESPACE leakage."
#endif
#define LOCTEXT_NAMESPACE "AAGX_Terrain"

AAGX_Terrain::AAGX_Terrain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

#if WITH_EDITOR && UE_VERSION_OLDER_THAN(5, 0, 0) == false
	// Actors that are spatially loaded (streamed in/out via world partitioning) may not reference
	// actors that are not. Since the ALanscape is not spatially loaded, the AGX_Terrain cannot be
	// either since we reference an ALandscape from it. Default value for all actors in OpenWorld is
	// true.
	bIsSpatiallyLoaded = false;
#endif

	SpriteComponent = CreateDefaultSubobject<UAGX_TerrainSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
	RootComponent = SpriteComponent;

	TerrainBounds = CreateDefaultSubobject<UAGX_HeightFieldBoundsComponent>(TEXT("TerrainBounds"));

	// Set render targets and niagara system from plugin by default to reduce manual steps when
	// using Terrain.
	auto AssignDefault = [](auto*& AssetRefProperty, const TCHAR* Path)
	{
		if (AssetRefProperty != nullptr)
			return;

		using Type = typename std::remove_reference<decltype(*AssetRefProperty)>::type;
		auto AssetFinder = ConstructorHelpers::FObjectFinder<Type>(Path);
		if (!AssetFinder.Succeeded())
		{
			UE_LOG(LogAGX, Warning, TEXT("Expected to find asset '%s' but it was not found."), *Path);
			return;
		}

		AssetRefProperty = AssetFinder.Object;
	};

	AssignDefault(
		LandscapeDisplacementMap,
		TEXT("TextureRenderTarget2D'/AGXUnreal/Terrain/Rendering/HeightField/"
			 "RT_LandscapeDisplacementMap.RT_LandscapeDisplacementMap'"));

	AssignDefault(
		ParticleSystemAsset, TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/"
								  "PS_SoilParticleSystem.PS_SoilParticleSystem'"));

	AssignDefault(
		TerrainParticlesDataMap,
		TEXT("TextureRenderTarget2D'/AGXUnreal/Terrain/Rendering/Particles/"
			 "RT_TerrainParticleData.RT_TerrainParticleData'"));
}

bool AAGX_Terrain::SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial)
{
	UAGX_TerrainMaterial* TerrainMaterialOrig = TerrainMaterial;
	TerrainMaterial = InTerrainMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeMaterial())
	{
		// Something went wrong, restore original TerrainMaterial.
		TerrainMaterial = TerrainMaterialOrig;
		return false;
	}

	return true;
}

void AAGX_Terrain::AddCollisionGroup(const FName& GroupName)
{
	if (GroupName.IsNone())
	{
		return;
	}

	if (CollisionGroups.Contains(GroupName))
		return;

	CollisionGroups.Add(GroupName);
	if (HasNative())
		NativeBarrier.AddCollisionGroup(GroupName);
}

void AAGX_Terrain::RemoveCollisionGroupIfExists(const FName& GroupName)
{
	if (GroupName.IsNone())
		return;

	auto Index = CollisionGroups.IndexOfByKey(GroupName);
	if (Index == INDEX_NONE)
		return;

	CollisionGroups.RemoveAt(Index);
	if (HasNative())
		NativeBarrier.RemoveCollisionGroup(GroupName);
}

UNiagaraComponent* AAGX_Terrain::GetSpawnedParticleSystemComponent()
{
	return ParticleSystemComponent;
}

int32 AAGX_Terrain::GetNumParticles() const
{
	if (!HasNative())
		return 0;

	if (HasNativeTerrainPager())
	{
		return static_cast<int32>(NativeTerrainPagerBarrier.GetNumParticles());
	}
	else
	{
		check(HasNative());
		return static_cast<int32>(NativeBarrier.GetNumParticles());
	}
}

void AAGX_Terrain::SetCreateParticles(bool CreateParticles)
{
	if (HasNative())
	{
		NativeBarrier.SetCreateParticles(CreateParticles);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	bCreateParticles = CreateParticles;
}

void AAGX_Terrain::SetEnableTerrainPaging(bool bEnabled)
{
	bEnableTerrainPaging = bEnabled;
}

bool AAGX_Terrain::GetEnableTerrainPaging() const
{
	return bEnableTerrainPaging;
}

bool AAGX_Terrain::GetCreateParticles() const
{
	if (HasNative())
	{
		return NativeBarrier.GetCreateParticles();
	}

	return bCreateParticles;
}

void AAGX_Terrain::SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds)
{
	if (HasNative())
	{
		NativeBarrier.SetDeleteParticlesOutsideBounds(DeleteParticlesOutsideBounds);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	bDeleteParticlesOutsideBounds = DeleteParticlesOutsideBounds;
}

bool AAGX_Terrain::GetDeleteParticlesOutsideBounds() const
{
	if (HasNative())
	{
		return NativeBarrier.GetDeleteParticlesOutsideBounds();
	}

	return bDeleteParticlesOutsideBounds;
}

void AAGX_Terrain::SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling)
{
	if (HasNative())
	{
		NativeBarrier.SetPenetrationForceVelocityScaling(InPenetrationForceVelocityScaling);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	PenetrationForceVelocityScaling = InPenetrationForceVelocityScaling;
}

double AAGX_Terrain::GetPenetrationForceVelocityScaling() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPenetrationForceVelocityScaling();
	}

	return PenetrationForceVelocityScaling;
}

void AAGX_Terrain::SetPenetrationForceVelocityScaling_BP(float InPenetrationForceVelocityScaling)
{
	SetPenetrationForceVelocityScaling(static_cast<double>(InPenetrationForceVelocityScaling));
}

float AAGX_Terrain::GetPenetrationForceVelocityScaling_BP() const
{
	return static_cast<float>(GetPenetrationForceVelocityScaling());
}

void AAGX_Terrain::SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume)
{
	if (HasNative())
	{
		NativeBarrier.SetMaximumParticleActivationVolume(InMaximumParticleActivationVolume);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	MaximumParticleActivationVolume = InMaximumParticleActivationVolume;
}

double AAGX_Terrain::GetMaximumParticleActivationVolume() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMaximumParticleActivationVolume();
	}

	return MaximumParticleActivationVolume;
}

void AAGX_Terrain::SetMaximumParticleActivationVolume_BP(float InMaximumParticleActivationVolume)
{
	SetMaximumParticleActivationVolume(static_cast<double>(InMaximumParticleActivationVolume));
}

float AAGX_Terrain::GetMaximumParticleActivationVolume_BP() const
{
	return static_cast<float>(GetMaximumParticleActivationVolume());
}

bool AAGX_Terrain::HasNative() const
{
	return NativeBarrier.HasNative() && (!bEnableTerrainPaging || HasNativeTerrainPager());
}

bool AAGX_Terrain::HasNativeTerrainPager() const
{
	return NativeTerrainPagerBarrier.HasNative();
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

FTerrainPagerBarrier* AAGX_Terrain::GetNativeTerrainPager()
{
	if (!NativeTerrainPagerBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeTerrainPagerBarrier;
}

const FTerrainPagerBarrier* AAGX_Terrain::GetNativeTerrainPager() const
{
	if (!NativeTerrainPagerBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeTerrainPagerBarrier;
}

namespace
{
	/**
	Calculates and returns the smallest base size of a square sized texture,
	such that the base size is evenly divisible by pixelsPerItem and has a square
	that is at least minNumItems x pixelsPerItem.
	*/
	int32 CalculateTextureBaseSize(int32 MinNumItems, int32 PixelsPerItem)
	{
		// Max size taken from UTextureRenderTarget2D::PostEditChangeProperty in
		// Engine/Source/Runtime/Engine/Private/TextureRenderTarget2D.cpp. Update here
		// if future versions of Unreal Engine allow larger render targets.
		const int32 MaxSide = 8192;
		const int32 NumPixels = MinNumItems * PixelsPerItem;
		int32 Side =
			FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(static_cast<double>(NumPixels))), 0, MaxSide);
		// We might not get a good side length on the first attempt, so search upwards until we
		// find one.
		for (; Side <= MaxSide; ++Side)
		{
			if ((Side % PixelsPerItem == 0) && (Side * Side >= NumPixels))
			{
				return Side;
			}
		}
		AGX_CHECK(!"CalculateTextureBaseSize failed");
		return 0;
	}

	bool ParticleDataRenderTargetValid(
		const UTextureRenderTarget2D& TerrainParticlesDataMap, int32 TextureBaseSize,
		int32 NumPixelsPerParticle)
	{
		const int32 NumTexels = TerrainParticlesDataMap.SizeX * TerrainParticlesDataMap.SizeY;
		const bool TargetLargeEnough = NumTexels >= TextureBaseSize * TextureBaseSize;
		const bool TargetSquare = TerrainParticlesDataMap.SizeX == TerrainParticlesDataMap.SizeY;
		const bool TargetSizeMultipleOfPpp =
			TerrainParticlesDataMap.SizeX % NumPixelsPerParticle == 0;
		return TargetLargeEnough && TargetSquare && TargetSizeMultipleOfPpp;
	}
}

#if WITH_EDITOR

void AAGX_Terrain::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void AAGX_Terrain::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

bool AAGX_Terrain::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (!HasNative())
		return SuperCanEditChange;

	const FName Prop = InProperty->GetFName();

	// Properties that should never be edited during Play.
	if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, SourceLandscape))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, Shovels))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, ParticleSystemAsset))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, LandscapeDisplacementMap))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, TerrainParticlesDataMap))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bEnableTerrainPaging))
		return false;
	else
		return SuperCanEditChange;
}

namespace AGX_Terrain_helpers
{
	void EnsureUseDynamicMaterialInstance(AAGX_Terrain& Terrain)
	{
		TArray<ALandscapeProxy*> StreamingProxies;
		if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Terrain.SourceLandscape))
		{
			for (TObjectIterator<ALandscapeStreamingProxy> It; It; ++It)
			{
				if (It->GetLandscapeActor() != Terrain.SourceLandscape)
					continue;

				StreamingProxies.Add(*It);
			}
		}

		auto IsUsingDynamicMaterialInstance = [&StreamingProxies](ALandscape& Landscape)
		{
			bool Res = Landscape.bUseDynamicMaterialInstance;
			for (auto Proxy : StreamingProxies)
				Res &= Proxy->bUseDynamicMaterialInstance;

			return Res;
		};

		if (Terrain.SourceLandscape == nullptr ||
			IsUsingDynamicMaterialInstance(*Terrain.SourceLandscape))
		{
			return;
		}

		FText AskEnableDynamicMaterial = LOCTEXT(
			"EnableDynamicMaterial?",
			"The selected Landscape does not have Use Dynamic Material Instance enabled, "
			"meaning that the material parameters for Landsacpe size and position cannot "
			"be set automatically. Should Use Dynamic Material Instance be enabled on the "
			"Landscape?");
		if (FAGX_NotificationUtilities::YesNoQuestion(AskEnableDynamicMaterial))
		{
			auto SetUseDynamicMaterialInstance = [](ALandscapeProxy& Proxy)
			{
				Proxy.bUseDynamicMaterialInstance = true;
				Proxy.Modify();
				Proxy.PostEditChange();
			};

			SetUseDynamicMaterialInstance(*Terrain.SourceLandscape);

			if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Terrain.SourceLandscape))
			{
				for (auto Proxy : StreamingProxies)
				{
					SetUseDynamicMaterialInstance(*Proxy);
				}
			}
		}
	}
}

void AAGX_Terrain::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, SourceLandscape),
		[](ThisClass* This) { AGX_Terrain_helpers::EnsureUseDynamicMaterialInstance(*This); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bCreateParticles),
		[](ThisClass* This) { This->SetCreateParticles(This->bCreateParticles); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bDeleteParticlesOutsideBounds), [](ThisClass* This)
		{ This->SetDeleteParticlesOutsideBounds(This->bDeleteParticlesOutsideBounds); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, PenetrationForceVelocityScaling), [](ThisClass* This)
		{ This->SetPenetrationForceVelocityScaling(This->PenetrationForceVelocityScaling); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, MaximumParticleActivationVolume), [](ThisClass* This)
		{ This->SetMaximumParticleActivationVolume(This->MaximumParticleActivationVolume); });

	// The MaxNumRenderParticles, ParticleSystemAsset, and TerrainParticlesDataMap Properties
	// have dependencies between them. The TerrainParticlesDataMap must be large enough to hold
	// data for all MaxNumRenderParticles particles, and whenever the TerrainParticlesDataMap
	// is changed, either in size or which asset is pointed to, the ParticleSystemAsset must be
	// recompiled.

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, MaxNumRenderParticles),
		[](ThisClass* This) { This->EnsureParticleDataRenderTargetSize(); });

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ParticleSystemAsset),
		[](ThisClass* This)
		{
			if (This->ParticleSystemAsset != nullptr)
			{
				This->ParticleSystemAsset->RequestCompile(true);
			}
		});

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(TerrainParticlesDataMap),
		[](ThisClass* This) { This->EnsureParticleDataRenderTargetSize(); });

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bEnableTerrainPaging),
		[](ThisClass* This) { This->SetEnableTerrainPaging(This->bEnableTerrainPaging); });
}

void AAGX_Terrain::EnsureParticleDataRenderTargetSize()
{
	if (!TerrainParticlesDataMap)
	{
		return;
	}

	const int32 TextureBaseSize =
		CalculateTextureBaseSize(MaxNumRenderParticles, NumPixelsPerParticle);
	if (TextureBaseSize == 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not find a render target size able to accomodate %d render particles for "
				 "AGX_Terrain '%s'."),
			MaxNumRenderParticles, *GetActorLabel());
		return;
	}

	if (!ParticleDataRenderTargetValid(
			*TerrainParticlesDataMap, TextureBaseSize, NumPixelsPerParticle))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain Particles Data Map used by Terrain '%s' doesn't have the required "
				 "size to hold the requested maximum number of render particles. "
				 "Requesting resize from %d x %d to %d x %d."),
			*this->GetActorLabel(), TerrainParticlesDataMap->SizeX, TerrainParticlesDataMap->SizeY,
			TextureBaseSize, TextureBaseSize);

		const bool DoResize = FAGX_NotificationUtilities::YesNoQuestion(FText::Format(
			LOCTEXT(
				"ResizeRenderTarget?",
				"Current render target for particle data is too small to hold the requested number "
				"of render particles. Automatically resize the render target to {0}x{1}?"),
			TextureBaseSize, TextureBaseSize));
		if (DoResize)
		{
			TerrainParticlesDataMap->ResizeTarget(TextureBaseSize, TextureBaseSize);
			FAGX_ObjectUtilities::SaveAsset(*TerrainParticlesDataMap);
		}
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

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		// Update the Displacement Map on each PostStepForward
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this](float)
					{
						if (bEnableDisplacementRendering)
						{
							UpdateDisplacementMap();
						}
					});
	}
}

void AAGX_Terrain::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	ClearDisplacementMap();
	ClearParticlesMap();
	if (HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			// @todo Figure out how to handle Terrain Materials. A Terrain Material can be
			// shared between many Terrains in theory. We only want to remove the Terrain
			// Material from the simulation if this Terrain is the last one using it. Some
			// reference counting may be needed.
			Simulation->Remove(*this);

			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.Remove(PostStepForwardHandle);
		}
	}

	if (HasNativeTerrainPager())
	{
		NativeTerrainPagerBarrier.ReleaseNative();
	}
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::Tick"));
	Super::Tick(DeltaTime);
	if (bEnableParticleRendering)
	{
		UpdateParticlesMap();
	}
}

bool AAGX_Terrain::FetchHeights(
	const FVector& WorldPosStart, int32 VertsX, int32 VertsY, TArray<float>& OutHeights)
{
	/*
	 * This function will be called by the native Terrain Pager from a worker thread, meaning we
	 * have to make sure that what we do here is thread safe. For example, we protect the
	 * OriginalHeights array here since it may be read from the main thread in
	 * UpdateDisplacementMap.
	 */

	if (SourceLandscape == nullptr || !HasNative())
		return false;

	const double QuadSizeX = SourceLandscape->GetActorScale().X;
	const double QuadSizeY = SourceLandscape->GetActorScale().Y;
	const FVector PosStartLocal =
		SourceLandscape->GetTransform().InverseTransformPositionNoScale(WorldPosStart);
	const int32 StartVertX = FMath::RoundToInt(PosStartLocal.X / QuadSizeX);
	const int32 StartVertY = FMath::RoundToInt(PosStartLocal.Y / QuadSizeY);

	const FVector NativePosLocal = SourceLandscape->GetTransform().InverseTransformPositionNoScale(
		GetNativeTransform().GetLocation());
	const int32 BoundsCornerMinX =
		FMath::RoundToInt(NativePosLocal.X / QuadSizeX) - NumVerticesX / 2;
	const int32 BoundsCornerMinY =
		FMath::RoundToInt(NativePosLocal.Y / QuadSizeY) - NumVerticesY / 2;
	const int32 BoundsCornerMaxX =
		FMath::RoundToInt(NativePosLocal.X / QuadSizeX) + NumVerticesX / 2;
	const int32 BoundsCornerMaxY =
		FMath::RoundToInt(NativePosLocal.Y / QuadSizeY) + NumVerticesY / 2;

	// Check that we are not asked to read outside the bounds.
	if (StartVertX < BoundsCornerMinX || StartVertY < BoundsCornerMinY ||
		StartVertX + VertsX - 1 > BoundsCornerMaxX || StartVertY + VertsY - 1 > BoundsCornerMaxY)
	{
		return false;
	}

	OutHeights.Reserve(VertsX * VertsY);

	{
		std::lock_guard<std::mutex> ScopedOrigHeightsLock(OriginalHeightsMutex);

		// AGX Dynamics coordinate systems are mapped with Y-axis flipped.
		for (int Y = StartVertY + VertsY - 1; Y >= StartVertY; Y--)
		{
			for (int X = StartVertX; X < StartVertX + VertsX; X++)
			{
				const FVector SamplePosLocal = FVector(
					static_cast<double>(X) * QuadSizeX, static_cast<double>(Y) * QuadSizeY, 0.0);
				const FVector SamplePosGlobal =
					SourceLandscape->GetTransform().TransformPositionNoScale(SamplePosLocal);

				if (auto Height = SourceLandscape->GetHeightAtLocation(SamplePosGlobal))
				{
					FVector HeightPointLocal =
						SourceLandscape->GetTransform().InverseTransformPositionNoScale(
							FVector(SamplePosGlobal.X, SamplePosGlobal.Y, *Height));
					OutHeights.Add(HeightPointLocal.Z);
					OriginalHeights
						[(X - BoundsCornerMinX) + (Y - BoundsCornerMinY) * NumVerticesX] =
							HeightPointLocal.Z;
				}
				else
				{
					UE_LOG(
						LogTemp, Warning,
						TEXT("Height read unsuccessful in Terrain. World sample pos: %s"),
						*SamplePosGlobal.ToString());
					OutHeights.Add(SourceLandscape->GetActorLocation().Z);
					OriginalHeights
						[(X - BoundsCornerMinX) + (Y - BoundsCornerMinY) * NumVerticesX] =
							SourceLandscape->GetActorLocation().Z;
				}
			}
		}
	}

	return true;
}

FTransform AAGX_Terrain::GetNativeTransform() const
{
	check(HasNative());

	if (bEnableTerrainPaging)
		return FTransform(
			NativeTerrainPagerBarrier.GetReferenceRotation(),
			NativeTerrainPagerBarrier.GetReferencePoint());
	else
		return FTransform(NativeBarrier.GetRotation(), NativeBarrier.GetPosition());
}

namespace
{
	UAGX_RigidBodyComponent* GetBodyComponent(
		AActor* OwningActor, const FString& BodyName, const TCHAR* TerrainName)
	{
		TArray<UAGX_RigidBodyComponent*> Bodies;
		OwningActor->GetComponents(Bodies, false);
		UAGX_RigidBodyComponent** It = Bodies.FindByPredicate(
			[BodyName](UAGX_RigidBodyComponent* Body) { return BodyName == Body->GetName(); });
		if (It == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot create shovel: Shovel Actor '%s' in terrain '%s' could not be created "
					 "because the configured shovel body '%s' does not exist in the shovel Actor."),
				*OwningActor->GetName(), TerrainName, *BodyName);

			return nullptr;
		}
		return *It;
	}

	template <typename TPtr>
	TPtr GetShovelComponent(UAGX_RigidBodyComponent& Body, const TCHAR* TerrainName)
	{
		auto RecursiveFind = [](const TArray<USceneComponent*>& Components, auto& recurse)
		{
			for (USceneComponent* Component : Components)
			{
				if (TPtr Match = Cast<std::remove_pointer_t<TPtr>>(Component))
				{
					return Match;
				}
				if (TPtr Match = recurse(Component->GetAttachChildren(), recurse))
				{
					return Match;
				}
			}
			return TPtr(nullptr);
		};

		TPtr Result = RecursiveFind(Body.GetAttachChildren(), RecursiveFind);
		if (Result == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to find Shovel Component '%s' in Rigid Body '%s'. Make sure it has "
					 "been added as a child to the Rigid Body."),
				*std::remove_pointer<TPtr>::type::StaticClass()->GetName(), *Body.GetName());
		}

		return Result;
	}

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
		UE_LOG(LogAGX, Error, TEXT("No source landscape selected for terrain %s."), *GetName());
		return;
	}

	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("BeginPlay called on a Terrain that has already been initialized."));
		return;
	}

	HeightFetcher.SetTerrain(this);

	if (!CreateNative())
	{
		return; // Logging done in CreateNative.
	}

	if (bEnableTerrainPaging)
	{
		if (!CreateNativeTerrainPager())
		{
			return; // Logging done in CreateNativeTerrainPager.
		}
	}

	CreateNativeShovels();
	AddTerrainPagerBodies();
	InitializeRendering();

	if (!UpdateNativeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeMaterial returned false in AGX_Terrain. "
				 "Ensure the selected Terrain Material is valid."));
	}
}

bool AAGX_Terrain::CreateNative()
{
	TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo> Bounds =
		TerrainBounds->GetLandscapeAdjustedBounds();
	if (!Bounds.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create Terrain native for '%s'; the given Terrain Bounds was invalid."),
			*GetName());
		return false;
	}

	const FVector StartPos = Bounds->Transform.TransformPositionNoScale(-Bounds->HalfExtent);

	FHeightFieldShapeBarrier HeightField = AGX_HeightFieldUtilities::CreateHeightField(
		*SourceLandscape, StartPos, Bounds->HalfExtent.X * 2.0, Bounds->HalfExtent.Y * 2.0,
		!bEnableTerrainPaging);

	NativeBarrier.AllocateNative(HeightField, MaxDepth);

	if (!NativeBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create Terrain native for '%s'. The Output log may include more "
				 "details."),
			*GetName());
		return false;
	}

	NativeBarrier.SetRotation(Bounds->Transform.GetRotation());
	NativeBarrier.SetPosition(Bounds->Transform.GetLocation());

	NumVerticesX =
		FMath::RoundToInt(Bounds->HalfExtent.X * 2.0 / SourceLandscape->GetActorScale().X) + 1;
	NumVerticesY =
		FMath::RoundToInt(Bounds->HalfExtent.Y * 2.0 / SourceLandscape->GetActorScale().Y) + 1;

	NativeBarrier.AddCollisionGroups(CollisionGroups);

	if (bEnableTerrainPaging)
	{
		OriginalHeights.SetNumZeroed(NumVerticesX * NumVerticesY);
	}
	else
	{
		AGX_CHECK(NumVerticesX == NativeBarrier.GetGridSizeX());
		AGX_CHECK(NumVerticesY == NativeBarrier.GetGridSizeY());
		OriginalHeights.Reserve(NumVerticesX * NumVerticesY);
		NativeBarrier.GetHeights(OriginalHeights, false);
	}

	// We must initialize CurrentHeights since we will only read height changes during runtime.
	CurrentHeights.Reserve(OriginalHeights.Num());
	CurrentHeights = OriginalHeights;

	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
	NativeBarrier.SetMaximumParticleActivationVolume(MaximumParticleActivationVolume);

	// Create the AGX Dynamics instance for the terrain.
	// Note that the AGX Dynamics Terrain messes with the solver parameters on add, parameters that
	// our user may have set explicitly. If so, re-set the user-provided settings.
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	int32 NumIterations = Simulation->GetNumPpgsIterations();

	if (!bEnableTerrainPaging)
	{
		// We add this Terrain to the Simulation here only if we are not using TerrainPaging.
		Simulation->Add(*this);
	}

	if (Simulation->bOverridePPGSIterations)
	{
		// We must check the override flag and not blindly re-set the value we read a few lines up
		// because when not overriding one should get the number of iterations set by the terrain,
		// not the number of iterations that is the default in the solver.
		Simulation->SetNumPpgsIterations(NumIterations);
	}
	else
	{
		// Not overriding the number of iterations, which means that the UAGX_Simulation instance
		// should be notified of the new current number of iterations set by the AGX Dynamics
		// terrain. Not using SetNumPpgsIterations because this code fixes a broken class invariant,
		// it does not move from one valid state to another, so lower-level fiddling is required.
		//
		// I don't like it.
		Simulation->NumPpgsIterations = Simulation->GetNative()->GetNumPpgsIterations();
	}

	return true;
}

bool AAGX_Terrain::CreateNativeTerrainPager()
{
	check(NativeBarrier.HasNative());
	check(!HasNativeTerrainPager());

	if (!bEnableTerrainPaging)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeTerrainPager called on Terrain '%s' which doesn't use Terrain "
				 "Paging."),
			*GetName());
		return false;
	}

	// Always set DeleteParticlesOutsideBounds to false if we are using Terrain Paging, otherwise
	// particles may be deleted when tiles are loaded and unloaded in an unexpected way. This will
	// be handled automatically by AGX Dynamics in the future.
	if (bDeleteParticlesOutsideBounds)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("DeleteParticlesOutsideBounds was set to true while using Terrain Paging. This "
				 "combination is not supported. DeleteParticlesOutsideBounds will be set to "
				 "false."));
		SetDeleteParticlesOutsideBounds(false);
	}

	const auto QuadSize = SourceLandscape->GetActorScale().X;
	const int32 TileNumVerticesSide =
		FMath::RoundToInt(TerrainPagingSettings.TileSize / QuadSize) + 1;
	const int32 TileOverlapVertices =
		FMath::RoundToInt(TerrainPagingSettings.TileOverlap / QuadSize);

	NativeTerrainPagerBarrier.AllocateNative(
		&HeightFetcher, NativeBarrier, TileNumVerticesSide, TileOverlapVertices, QuadSize,
		MaxDepth);

	if (!HasNativeTerrainPager())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create Terrain Pager native for '%s'. The Output log may include more "
				 "details."),
			*GetName());
		return false;
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	Simulation->Add(*this);
	return true;
}

void AAGX_Terrain::CreateNativeShovels()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeShovels called on Terrain '%s' which doesn't have a native "
				 "representation."),
			*GetName());
	}

	auto AddShovel = [this](FShovelBarrier& ShovelBarrier, const FAGX_Shovel& Shovel) -> bool
	{
		if (bEnableTerrainPaging)
		{
			return NativeTerrainPagerBarrier.AddShovel(
				ShovelBarrier, Shovel.RequiredRadius, Shovel.PreloadRadius);
		}
		else
		{
			return NativeBarrier.AddShovel(ShovelBarrier);
		}
	};

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
		UAGX_RigidBodyComponent* Body = ::GetBodyComponent(Actor, Shovel.BodyName, *GetName());
		if (Body == nullptr)
		{
			// Error message printed by GetBodyComponent.
			continue;
		}
		UAGX_TopEdgeComponent* TopEdge = GetShovelComponent<decltype(TopEdge)>(*Body, *GetName());
		UAGX_CuttingEdgeComponent* CuttingEdge =
			GetShovelComponent<decltype(CuttingEdge)>(*Body, *GetName());
		UAGX_CuttingDirectionComponent* CuttingDirection =
			GetShovelComponent<decltype(CuttingDirection)>(*Body, *GetName());

		if (TopEdge == nullptr || CuttingEdge == nullptr || CuttingDirection == nullptr)
		{
			// GetShovelComponent is responsible for printing the error message.
			continue;
		}

		FShovelBarrier ShovelBarrier;
		FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
		const FTransform WorldToBody = Body->GetComponentTransform().Inverse();
		FTwoVectors TopEdgeLine = TopEdge->GetInLocal(WorldToBody);
		FTwoVectors CuttingEdgeLine = CuttingEdge->GetInLocal(WorldToBody);

		// AGX Dynamics always expects a normalized Cutting Direction vector.
		const FVector CuttingDirectionVector =
			WorldToBody.TransformVector(CuttingDirection->GetVectorDirection()).GetSafeNormal();

		ShovelBarrier.AllocateNative(
			*BodyBarrier, TopEdgeLine, CuttingEdgeLine, CuttingDirectionVector);

		FAGX_Shovel::UpdateNativeShovelProperties(ShovelBarrier, Shovel);

		bool Added = AddShovel(ShovelBarrier, Shovel);
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
			Added = AddShovel(ShovelBarrier, Shovel);
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

void AAGX_Terrain::AddTerrainPagerBodies()
{
	if (!HasNativeTerrainPager())
		return;

	for (FAGX_TerrainPagingBodyReference& TrackedBody : TerrainPagingSettings.TrackedRigidBodies)
	{
		UAGX_RigidBodyComponent* Body = TrackedBody.RigidBody.GetRigidBody();
		if (Body == nullptr)
			continue;

		FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
		if (BodyBarrier == nullptr)
			continue;

		NativeTerrainPagerBarrier.AddRigidBody(
			*Body->GetNative(), TrackedBody.RequiredRadius, TrackedBody.PreloadRadius);
	}
}

void AAGX_Terrain::InitializeRendering()
{
	if (bEnableDisplacementRendering)
	{
		InitializeDisplacementMap();
	}
	if (bEnableParticleRendering)
	{
		ParticleSystemInitialized = InitializeParticleSystem();
	}

	UpdateLandscapeMaterialParameters();
}

bool AAGX_Terrain::UpdateNativeMaterial()
{
	if (!HasNative())
		return false;

	if (TerrainMaterial == nullptr)
	{
		GetNative()->ClearMaterial();
		return true;
	}

	// Set TerrainMaterial
	UAGX_TerrainMaterial* TerrainMaterialInstance =
		static_cast<UAGX_TerrainMaterial*>(TerrainMaterial->GetOrCreateInstance(GetWorld()));
	check(TerrainMaterialInstance);

	if (TerrainMaterial != TerrainMaterialInstance)
	{
		TerrainMaterial = TerrainMaterialInstance;
	}

	FTerrainMaterialBarrier* TerrainMaterialBarrier =
		TerrainMaterialInstance->GetOrCreateTerrainMaterialNative(GetWorld());
	check(TerrainMaterialBarrier);

	GetNative()->SetTerrainMaterial(*TerrainMaterialBarrier);

	// Set ShapeMaterial
	FShapeMaterialBarrier* MaterialBarrier =
		TerrainMaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
	check(MaterialBarrier);

	GetNative()->SetShapeMaterial(*MaterialBarrier);
	return true;
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

	if (LandscapeDisplacementMap->SizeX != NumVerticesX ||
		LandscapeDisplacementMap->SizeY != NumVerticesY)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("The size of the Displacement Map render target (%dx%d) for "
				 "AGX Terrain '%s' does not match the vertices in the Terrain (%dx%d). "
				 "Resizing the displacement map."),
			LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY, *GetName(),
			NumVerticesX, NumVerticesY);

		LandscapeDisplacementMap->ResizeTarget(NumVerticesX, NumVerticesY);
	}
	if (LandscapeDisplacementMap->SizeX != NumVerticesX ||
		LandscapeDisplacementMap->SizeY != NumVerticesY)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Landscape displacement map for terrain '%s' could not be resized. "
				 "There may be rendering issues."),
			*GetName(), LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY);
	}

	DisplacementData.SetNum(NumVerticesX * NumVerticesY);
	DisplacementMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, NumVerticesX, NumVerticesY));

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
	if (!HasNative())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::UpdateDisplacementMap"));

	TArray<std::tuple<int32, int32>> ModifiedVertices;
	if (bEnableTerrainPaging)
	{
		ModifiedVertices = NativeTerrainPagerBarrier.GetModifiedHeights(
			CurrentHeights, NumVerticesX, NumVerticesY);
	}
	else
	{
		NativeBarrier.GetHeights(CurrentHeights, true);
		ModifiedVertices = NativeBarrier.GetModifiedVertices();
	}

	{
		std::lock_guard<std::mutex> ScopedOrigHeightsLock(OriginalHeightsMutex);
		for (const auto& VertexTuple : ModifiedVertices)
		{
			const int32 VertX = std::get<0>(VertexTuple);
			const int32 VertY = std::get<1>(VertexTuple);
			const int32 Index = VertX + VertY * NumVerticesX;
			const float HeightChange = CurrentHeights[Index] - OriginalHeights[Index];
			DisplacementData[Index] = static_cast<FFloat16>(HeightChange);
		}
	}

	const uint32 BytesPerPixel = sizeof(FFloat16);
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
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
	if (!HasNative())
	{
		return;
	}
	if (DisplacementMapRegions.Num() == 0)
	{
		return;
	}

	const uint32 BytesPerPixel = sizeof(FFloat16);
	for (FFloat16& Displacement : DisplacementData)
	{
		Displacement = FFloat16();
	}
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}

bool AAGX_Terrain::InitializeParticleSystem()
{
	return InitializeParticleSystemComponent() && InitializeParticlesMap();
}

bool AAGX_Terrain::InitializeParticleSystemComponent()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain '%s' does not have a particle system, cannot render particles"),
			*GetName());
		return false;
	}

	// It is important that we attach the ParticleSystemComponent using "KeepRelativeOffset" so that
	// it's world position becomes the same as the Terrain's. Otherwise it will be spawned at
	// the world origin which in turn may result in particles being culled and not rendered if the
	// terrain is located far away from the world origin (see Fixed Bounds in the Particle System).
	ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
		FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset, false,
#if UE_VERSION_OLDER_THAN(4, 24, 0)
		EPSCPoolMethod::None
#else
		ENCPoolMethod::None
#endif
	);
#if WITH_EDITORONLY_DATA
	ParticleSystemComponent->bVisualizeComponent = true;
#endif

	return true;
}

bool AAGX_Terrain::InitializeParticlesMap()
{
	if (TerrainParticlesDataMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("No particles data map configured for terrain '%s'. Terrain rendering will not "
				 "include particles."),
			*GetName());
		return false;
	}

	if (TerrainParticlesDataMap->GetFormat() != EPixelFormat::PF_A32B32G32R32F)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The particle data map pixel format for the terrain '%s' must be RGBA32F."),
			*GetName());
		return false;
	}

	// Finds the closest fitting base size of a square sized texture such that it has room for all
	// particles data and such that the base size is a multiple of number of pixels per particle
	// (because we do not want a row-break in the middle of a particle element, since that increases
	// complexity of the Niagara Module Script).
	const int32 TextureBaseSize =
		CalculateTextureBaseSize(MaxNumRenderParticles, NumPixelsPerParticle);
	if (TextureBaseSize == 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not find a render target size able to accomodate %d render particles for "
				 "AGX_Terrain '%s'. Terrain rendering will not include particles."),
			MaxNumRenderParticles, *GetLabelSafe(this));
		return false;
	}
	check(TextureBaseSize % NumPixelsPerParticle == 0);
	check(TextureBaseSize * TextureBaseSize >= MaxNumRenderParticles * NumPixelsPerParticle);
	if (!ParticleDataRenderTargetValid(
			*TerrainParticlesDataMap, TextureBaseSize, NumPixelsPerParticle))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The size of the particle data render target (%dx%d) for "
				 "AGX Terrain '%s' has a size that does not meet the requirements to hold the \n"
				 "terrain particles data. It must be square, be at least (%dx%d) in size and have "
				 "side size that is a multiple of %d. Please resize the displacement map. The \n"
				 "recommended size given current 'Max Num Render Particles' is (%dx%d). Note \n"
				 "that you may have to recompile the Niagara Particle System after changing the "
				 "size of the particle data render taget."),
			TerrainParticlesDataMap->SizeX, TerrainParticlesDataMap->SizeY, *GetName(),
			TextureBaseSize, TextureBaseSize, NumPixelsPerParticle, TextureBaseSize,
			TextureBaseSize);

		return false;
	}

	return true;
}

void AAGX_Terrain::UpdateParticlesMap()
{
	if (!ParticleSystemInitialized)
	{
		return;
	}

	if (!bEnableTerrainPaging && !HasNative())
	{
		return;
	}

	if (bEnableTerrainPaging && !HasNativeTerrainPager())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::UpdateParticlesMap"));

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

	const FParticleData ParticleData = bEnableTerrainPaging
										   ? NativeTerrainPagerBarrier.GetParticleData()
										   : NativeBarrier.GetParticleData();
	const TArray<FVector>& Positions = ParticleData.Positions;
	const TArray<float>& Radii = ParticleData.Radii;
	const TArray<FQuat>& Rotations = ParticleData.Rotations;

	AGX_CHECK(Positions.Num() == Radii.Num());
	AGX_CHECK(Positions.Num() == Rotations.Num());

	int32 NumParticles = FMath::Min(Positions.Num(), MaxNumParticles);
	ParticleSystemComponent->SetVariableInt(FName(TEXT("User.TargetParticleCount")), NumParticles);

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
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
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
	if (!HasNative())
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
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
		*TerrainParticlesDataMap, 1, ParticlesDataMapRegions.GetData(),
		ResolutionX * NumBytesPerPixel, NumBytesPerPixel, PixelData, false);
}

void AAGX_Terrain::UpdateLandscapeMaterialParameters()
{
	if (!IsValid(SourceLandscape) || GetWorld() == nullptr || !GetWorld()->IsGameWorld())
	{
		return;
	}

	// Set scalar material parameters for Landscape size and position.
	// It is the Landscape material's responsibility to declare and implement displacement map
	// sampling and passing on to World Position Offset.

	const auto QuadSideSizeX = SourceLandscape->GetActorScale().X;
	const auto QuadSideSizeY = SourceLandscape->GetActorScale().Y;

	// This assumes that the Terrain and Landscape resolution (quad size) is the same.
	const double TerrainSizeX = static_cast<double>(NumVerticesX - 1) * QuadSideSizeX;
	const double TerrainSizeY = static_cast<double>(NumVerticesY - 1) * QuadSideSizeY;

	const FVector TerrainCenterGlobal = NativeBarrier.GetPosition();

	const FVector TerrainCenterLocal =
		SourceLandscape->GetActorTransform().InverseTransformPositionNoScale(TerrainCenterGlobal);
	const FVector TerrainCornerLocal =
		TerrainCenterLocal - FVector(TerrainSizeX / 2.0, TerrainSizeY / 2.0, 0.0);
	const FVector TerrainCornerGlobal =
		SourceLandscape->GetActorTransform().TransformPositionNoScale(TerrainCornerLocal);

	const double PositionX = TerrainCornerGlobal.X;
	const double PositionY = TerrainCornerGlobal.Y;

	auto SetLandscapeMaterialParameters = [=](ALandscapeProxy& Proxy)
	{
		// Parameter for materials supporting only square Landscape.
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSize", static_cast<float>(TerrainSizeX));
		// Parameters for materials supporting rectangular Landscape.
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSizeX", static_cast<float>(TerrainSizeX));
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSizeY", static_cast<float>(TerrainSizeY));
		// Parameters for Landscape position.
		Proxy.SetLandscapeMaterialScalarParameterValue("TerrainPositionX", PositionX);
		Proxy.SetLandscapeMaterialScalarParameterValue("TerrainPositionY", PositionY);
	};

	SetLandscapeMaterialParameters(*SourceLandscape);

	if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*SourceLandscape))
	{
		// There might be a better way to get all LandscapeStreamingProxies directly from the
		// SourceLandscape, but I have not found any. This is likely slower than any such methods,
		// but this is not extremely time critical since this function is called only once on Play.
		// If a better way if getting them is found in the future, this can be replaced.
		for (TObjectIterator<ALandscapeStreamingProxy> It; It; ++It)
		{
			if (It->GetLandscapeActor() != SourceLandscape)
				continue;

			SetLandscapeMaterialParameters(**It);
		}
	}
}

void AAGX_Terrain::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::HeightFieldUsesBounds))
	{
		TerrainBounds->bInfiniteBounds = true;
	}

	if (SpriteComponent == nullptr && RootComponent == nullptr &&
		ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainCGDisablerCMRegistrarViewporIcons))
	{
		SpriteComponent = CreateDefaultSubobject<UAGX_TerrainSpriteComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());
		RootComponent = SpriteComponent;
	}
}

#undef LOCTEXT_NAMESPACE
