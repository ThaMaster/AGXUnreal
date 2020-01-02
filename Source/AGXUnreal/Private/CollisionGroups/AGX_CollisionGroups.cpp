#include "AGX_CollisionGroups.h"
#include "AGX_ShapeComponent.h"

UAGX_CollisionGroups::UAGX_CollisionGroups()
{
	PrimaryComponentTick.bCanEverTick = false;
	CollisionGroupsLastChange = CollisionGroups;
}

void UAGX_CollisionGroups::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroups, CollisionGroups)))
	{
		ApplyCollisionGroupChangesToAllChildren(PropertyChangedEvent);
	}
}

void UAGX_CollisionGroups::ApplyCollisionGroupChangesToAllChildren(FPropertyChangedEvent & PropertyChangedEvent)
{
	FName PropertyName = GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroups, CollisionGroups);
	int32 ChangedArrayIndex = PropertyChangedEvent.GetArrayIndex(PropertyName.ToString());
	EPropertyChangeType::Type ChangeType = PropertyChangedEvent.ChangeType;

	AActor* Parent = GetOwner();
	TArray<AActor*> ActorComponentsTree;
	TArray<AActor*> CurrentLevel;

	// Set Parent as root node of the tree
	CurrentLevel.Add(Parent);

	GetActorsTree(CurrentLevel, ActorComponentsTree);

	for (AActor* Actor : ActorComponentsTree)
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

void UAGX_CollisionGroups::GetActorsTree(
	TArray<AActor*> CurrentLevel, TArray<AActor*>& ActorTree)
{
	for (AActor* Actor : CurrentLevel)
	{
		ActorTree.Add(Actor);

		TArray<AActor*> NextLevel;
		Actor->GetAttachedActors(NextLevel);
		GetActorsTree(NextLevel, ActorTree);
	}
}
