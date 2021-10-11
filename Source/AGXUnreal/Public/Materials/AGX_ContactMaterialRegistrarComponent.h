#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_ContactMaterialRegistrarComponent.generated.h"

class UAGX_ContactMaterialBase;
class UAGX_ContactMaterialInstance;

/**
 * Defines which AGX Contact Materials should be used by the owning level.
 *
 * Note that this Component does not yet support in-game changes to the Contact Materials array.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent), HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_ContactMaterialRegistrarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_ContactMaterialRegistrarComponent();

	/**
	 * User defined AGX Contact Materials to use in this level.
	 *
	 * It is not allowed to modify this array in-game.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Manager")
	TArray<UAGX_ContactMaterialBase*> ContactMaterials;

	// ~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	// ~ End UActorComponent Interface

private:
	void RemoveContactMaterial(UAGX_ContactMaterialInstance* Instance, EEndPlayReason::Type Reason);
};
