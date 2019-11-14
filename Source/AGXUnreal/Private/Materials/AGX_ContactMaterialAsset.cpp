// Fill out your copyright notice in the Description page of Project Settings.


#include "Materials/AGX_ContactMaterialAsset.h"

#include "Classes/Engine/World.h"

#include "Materials/AGX_ContactMaterialInstance.h"


UAGX_ContactMaterialInstance* UAGX_ContactMaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ContactMaterialInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ContactMaterialInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
};

UAGX_ContactMaterialAsset* UAGX_ContactMaterialAsset::GetAsset()
{
	return this;
}