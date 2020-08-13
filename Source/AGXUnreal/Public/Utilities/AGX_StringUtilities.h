#pragma once

#include "CoreMinimal.h"

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
