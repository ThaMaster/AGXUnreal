// Fill out your copyright notice in the Description page of Project Settings.

#include "Materials/AGX_MaterialBase.h"


#include "AGX_LogCategory.h"
#include "AGX_MaterialInstance.h"

#include "Classes/Engine/World.h"

UAGX_MaterialInstance* UAGX_MaterialBase::GetOrCreateInstance(UWorld* PlayingWorld, UAGX_MaterialBase*& Property)
{
	if (Property == nullptr || PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		return nullptr;
	}

	UAGX_MaterialInstance* Instance = Property->GetOrCreateInstance(PlayingWorld);

	if (Instance != Property)
	{
		UE_LOG(
			LogAGX, Log, TEXT("UAGX_MaterialBase::GetOrCreateInstance is swapping a property (to \"%s\" from \"%s\")."),
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
