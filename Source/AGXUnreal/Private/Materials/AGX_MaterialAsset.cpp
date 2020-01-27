// Fill out your copyright notice in the Description page of Project Settings.

#include "Materials/AGX_MaterialAsset.h"

#include "Engine/World.h"

#include "Materials/AGX_MaterialInstance.h"

UAGX_MaterialInstance* UAGX_MaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_MaterialInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_MaterialInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
};

UAGX_MaterialAsset* UAGX_MaterialAsset::GetAsset()
{
	return this;
}