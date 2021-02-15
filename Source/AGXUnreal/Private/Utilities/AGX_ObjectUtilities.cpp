#include "Utilities/AGX_ObjectUtilities.h"

void FAGX_ObjectUtilities::GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors)
{
	TArray<AActor*> CurrentLevel;

	// Set Parent as root node of the tree
	CurrentLevel.Add(Parent);

	GetActorsTree(CurrentLevel, ChildActors);

	// Remove the parent itself from the ChildActors array
	ChildActors.Remove(Parent);
}

void FAGX_ObjectUtilities::GetActorsTree(
	const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors)
{
	for (AActor* Actor : CurrentLevel)
	{
		ChildActors.Add(Actor);

		TArray<AActor*> NextLevel;
		Actor->GetAttachedActors(NextLevel);
		GetActorsTree(NextLevel, ChildActors);
	}
}
