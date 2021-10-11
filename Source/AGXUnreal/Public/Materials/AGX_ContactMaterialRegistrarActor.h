#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ContactMaterialRegistrarActor.generated.h"

class UAGX_ContactMaterialRegistrarComponent;

/**
 * Defines which AGX Contact Materials should be used by the owning level.
 *
 * Note that this class does not yet support in-game changes to the Contact Materials array.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API AAGX_ContactMaterialRegistrarActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_ContactMaterialRegistrarActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_ContactMaterialRegistrarComponent* ContactMaterialRegistrarComponent;
};
