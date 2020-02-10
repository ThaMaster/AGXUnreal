#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"

class AActor;

class FAGX_ObjectUtilities
{
public:

	/**
	* Returns array containing all child actors (and all their child actors)
	* recursively, attached to the Parent actor passed as input argument.
	* The parent itself is not included in the set.
	*/
	static void GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors);

private:
	static void GetActorsTree(const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors);
};
