// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

// Standard library includes.
#include <algorithm>

/// @todo Consider making these not FORCEINLINE, wrapped in a namespace, and move to .cpp.
/// @note There may be native Unreal Engine versions of these. Use those instead, where available.

FORCEINLINE FName GetFNameSafe(const UObjectBase* Object)
{
	if (Object == nullptr)
	{
		return NAME_None;
	}
	else
	{
		return Object->GetFName();
	}
}

#if !UE_VERSION_OLDER_THAN(4, 25, 0)
FORCEINLINE FName GetFNameSafe(const FField* Field)
{
	if (Field == nullptr)
	{
		return NAME_None;
	}
	else
	{
		return Field->GetFName();
	}
}
#endif

FORCEINLINE FString GetLabelSafe(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return TEXT("(none)");
	}
#if WITH_EDITOR
	return Actor->GetActorLabel();
#else
	return Actor->GetName();
#endif
}

FORCEINLINE bool ContainsOnlyIntegers(const FString& str)
{
	return std::all_of(str.begin(), str.end(), TChar<TCHAR>::IsDigit);
}
