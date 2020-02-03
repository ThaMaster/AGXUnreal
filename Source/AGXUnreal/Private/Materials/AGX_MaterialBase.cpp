// Fill out your copyright notice in the Description page of Project Settings.

#include "Materials/AGX_MaterialBase.h"

#include "AGX_LogCategory.h"
#include "Materials/AGX_ShapeMaterialInstance.h"

#include "Engine/World.h"

UAGX_ShapeMaterialInstance* UAGX_MaterialBase::GetOrCreateShapeMaterialInstance(
	UWorld* PlayingWorld, UAGX_MaterialBase* Property)
{
	if (Property == nullptr || PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		return nullptr;
	}

	UAGX_ShapeMaterialInstance* Instance = Property->GetOrCreateShapeMaterialInstance(PlayingWorld);

	if (Instance != Property)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("UAGX_MaterialBase::GetOrCreateShapeMaterialInstance is swapping a property (to "
				 "\"%s\" from "
				 "\"%s\")."),
			*GetNameSafe(Instance), *GetNameSafe(Property));

		Property = Instance;
	}

	return Instance;
}

UAGX_MaterialBase::~UAGX_MaterialBase()
{
}

void UAGX_MaterialBase::CopyProperties(const UAGX_MaterialBase* Source)
{
	if (Source)
	{
		Bulk = Source->Bulk;
		Surface = Source->Surface;
	}
}
