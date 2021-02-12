#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

FORCEINLINE FName GetFNameSafe(const UObjectBase* Object)
{
	if (Object == NULL)
	{
		return NAME_None;
	}
	else
	{
		return Object->GetFName();
	}
}

#if !UE_VERSION_OLDER_THAN(4,25,0)
FORCEINLINE FName GetFNameSafe(const FField* Field)
{
	if (Field == NULL)
	{
		return NAME_None;
	}
	else
	{
		return Field->GetFName();
	}
}
#endif
