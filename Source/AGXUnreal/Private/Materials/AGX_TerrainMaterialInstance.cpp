#include "Materials/AGX_TerrainMaterialInstance.h"

#include "Engine/World.h"

#include "Materials/AGX_TerrainMaterialAsset.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"

UAGX_TerrainMaterialInstance::~UAGX_TerrainMaterialInstance()
{
}

FTerrainMaterialBarrier* UAGX_TerrainMaterialInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

UAGX_TerrainMaterialInstance* UAGX_TerrainMaterialInstance::GetOrCreateTerrainMaterialInstance(
	UWorld* PlayingWorld)
{
	return this;
}

UAGX_TerrainMaterialInstance* UAGX_TerrainMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TerrainMaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_TerrainMaterialInstance";

	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_TerrainMaterialInstance::CreateFromAsset is creating an instance "
			 "named \"%s\" (from asset  %s)."),
		*InstanceName, *Source->GetName());

	UAGX_TerrainMaterialInstance* NewInstance = NewObject<UAGX_TerrainMaterialInstance>(
		Outer, UAGX_TerrainMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	// Copy both the AGX_MaterialBase properties (bulk, surface) and the terrain specific properties
	NewInstance->CopyProperties(Source);
	NewInstance->Terrain = Source->Terrain;

	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}


UAGX_ShapeMaterialInstance * UAGX_TerrainMaterialInstance::GetOrCreateShapeMaterialInstance(UWorld * PlayingWorld)
{
	// Invalid call, GetOrCreateShapeMaterialInstance should never be called on an
	// AGX_TerrainMaterialInstance
	return nullptr;
}

void UAGX_TerrainMaterialInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FTerrainMaterialBarrier());

	NativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasNative());

	UpdateNativeProperties();
}

bool UAGX_TerrainMaterialInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

FTerrainMaterialBarrier* UAGX_TerrainMaterialInstance::GetNative()
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

void UAGX_TerrainMaterialInstance::UpdateNativeProperties()
{
	if (HasNative())
	{
		NativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));
		NativeBarrier->SetDensity(Terrain.TerrainDensity);
	}
}
