#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

/// @todo Consider making these not FORCEINLINE, wrapped in a namespace, and move to .cpp.

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
