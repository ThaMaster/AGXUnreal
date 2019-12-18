#include "AGX_CollisionGroups.h"

#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "..\Public\AGX_CollisionGroups.h"

#define LOCTEXT_NAMESPACE "AAGX_CollisionGroups"

AAGX_CollisionGroups::AAGX_CollisionGroups(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> ViewportIconTextureObject;
			FName Id;
			FText Name;

			FConstructorStatics()
				: ViewportIconTextureObject(TEXT("/Engine/EditorResources/S_Note"))
				, Id(TEXT("AGX_CollisionGroups"))
				, Name(LOCTEXT("ViewportIcon", "AGX Collision Groups"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (GetSpriteComponent())
		{
			GetSpriteComponent()->Sprite = ConstructorStatics.ViewportIconTextureObject.Get();
			GetSpriteComponent()->SpriteInfo.Category = ConstructorStatics.Id;
			GetSpriteComponent()->SpriteInfo.DisplayName = ConstructorStatics.Name;
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void AAGX_CollisionGroups::BeginPlay()
{
	AddCollisionGroupPairsToSimulation();
}

void AAGX_CollisionGroups::AddCollisionGroupPairsToSimulation()
{
	if (DisabledCollisionGroups.Num() > 0)
	{
		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(GetWorld());

		for (auto& collisionGroupPair : DisabledCollisionGroups)
		{
			Simulation->SetDisableCollisionGroupPair(
				collisionGroupPair.Group1, collisionGroupPair.Group2);
		}
	}
}

#undef LOCTEXT_NAMESPACE
