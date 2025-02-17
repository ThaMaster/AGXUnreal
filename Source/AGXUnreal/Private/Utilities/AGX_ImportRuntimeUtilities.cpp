// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImportRuntimeUtilities.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"

void FAGX_ImportRuntimeUtilities::WriteSessionGuid(
	UActorComponent& Component, const FGuid& SessionGuid)
{
	Component.ComponentTags.Empty();
	Component.ComponentTags.Add(*SessionGuid.ToString());
}

void FAGX_ImportRuntimeUtilities::WriteSessionGuidToAssetType(
	UObject& Object, const FGuid& SessionGuid)
{
	UMetaData* MetaData = Object.GetOutermost()->GetMetaData();
	MetaData->SetValue(&Object, TEXT("AGX_ImportSessionGuid"), *SessionGuid.ToString());
}

void FAGX_ImportRuntimeUtilities::OnComponentCreated(
	UActorComponent& Component, AActor& Owner, const FGuid& SessionGuid)
{
	WriteSessionGuid(Component, SessionGuid);
	Component.SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(&Component);
}

void FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(UObject& Object, const FGuid& SessionGuid)
{
	UMetaData* MetaData = Object.GetOutermost()->GetMetaData();
	MetaData->SetValue(&Object, TEXT("AGX_ImportSessionGuid"), *SessionGuid.ToString());
}
