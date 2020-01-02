#include "AGX_CollisionGroups.h"

#include "AGX_ShapeComponent.h"
#include "AGX_ObjectUtilities.h"
#include "AGX_LogCategory.h"

UAGX_CollisionGroups::UAGX_CollisionGroups()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_CollisionGroups::ForceRefreshChildShapes()
{
	UE_LOG(LogAGX, Log, TEXT("Force refresh shapes called."));

	AActor* Parent = GetOwner();
	TArray<AActor*> AllActors;
	FAGX_ObjectUtilities::GetChildActorsOfActor(Parent, AllActors);

	// The Parent must be processed as well.
	AllActors.Add(Parent);

	for (AActor* Actor : AllActors)
	{
		TArray<UAGX_ShapeComponent*> ChildrenShapeComponents;
		Actor->GetComponents(ChildrenShapeComponents, true);

		for (UAGX_ShapeComponent* ShapeComponent : ChildrenShapeComponents)
		{
			for (FName CollisionGroup : CollisionGroups)
			{
				// Note: duplicates will be ignored.
				ShapeComponent->AddCollisionGroup(CollisionGroup);
			}
		}
	}
}

void UAGX_CollisionGroups::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroups, CollisionGroups)))
	{
		ApplyCollisionGroupChanges(PropertyChangedEvent);
	}
}

void UAGX_CollisionGroups::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// This cannot be done in the constructor since the UPROPERTIES
	// has not yet been initialized at that point.
	CollisionGroupsLastChange = CollisionGroups;
}

void UAGX_CollisionGroups::ApplyCollisionGroupChanges(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroups, CollisionGroups);
	int32 ChangedArrayIndex = PropertyChangedEvent.GetArrayIndex(PropertyName.ToString());
	EPropertyChangeType::Type ChangeType = PropertyChangedEvent.ChangeType;

	AActor* Parent = GetOwner();
	TArray<AActor*> AllActors;
	FAGX_ObjectUtilities::GetChildActorsOfActor(Parent, AllActors);

	// The Parent must be processed as well.
	AllActors.Add(Parent);

	for (AActor* Actor : AllActors)
	{
		TArray<UAGX_ShapeComponent*> ChildrenShapeComponents;
		Actor->GetComponents(ChildrenShapeComponents, true);

		for (UAGX_ShapeComponent* ShapeComponent : ChildrenShapeComponents)
		{
			switch (ChangeType)
			{
				case EPropertyChangeType::ArrayAdd:
					ShapeComponent->AddCollisionGroup(CollisionGroups[ChangedArrayIndex]);
					break;
				case EPropertyChangeType::ArrayRemove:
					ShapeComponent->TryRemoveCollisionGroup(
						CollisionGroupsLastChange[ChangedArrayIndex]);
					break;
				case EPropertyChangeType::ArrayClear:
				{
					for (int i = 0; i < CollisionGroupsLastChange.Num(); i++)
					{
						ShapeComponent->TryRemoveCollisionGroup(CollisionGroupsLastChange[i]);
					}

					break;
				}
				case EPropertyChangeType::ValueSet: // Value changed.
				{
					// Remove old collision group and add new collision group.
					ShapeComponent->TryRemoveCollisionGroup(
						CollisionGroupsLastChange[ChangedArrayIndex]);

					ShapeComponent->AddCollisionGroup(CollisionGroups[ChangedArrayIndex]);

					break;
				}
				default:
					// Non implemented change type, do nothing.
					break;
			}
		}
	}

	CollisionGroupsLastChange = CollisionGroups;
}
