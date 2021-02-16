#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "AGX_ContactMaterialAsset.generated.h"

class UAGX_ContactMaterialInstance;

/**
 * Defines detailed properties for contacts between a specific pair of AGX Materials.
 *
 * This overrides most of the two AGX Material's individual settings (except for mass related ones).
 *
 * It recommended to use AGX Contact Materials, for most realistic simulation results.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ContactMaterialAsset : public UAGX_ContactMaterialBase
{
	/// \note Class comment above is used as tooltip in Content Browser etc, so trying to keep it
	/// simple and user-centered, while providing a more programmer-aimed comment below.

	// Represents Contact Material properties as an in-Editor asset that can be serialized to disk.
	//
	// Has no connection with the actual native AGX Contact Material.Instead, its sibling class
	// UAGX_ContactMaterialInstance handles all interaction with the actual native AGX Contact
	// Material.Therefore, all in - game objects with Uproperty Contact Material pointers need to
	// swap their pointers to in - game UAGX_ContactMaterialInstances using the static function
	// UAGX_ContactMaterialBase::GetOrCreateInstance.

	GENERATED_BODY()

public:
	virtual UAGX_ContactMaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;

private:
	virtual UAGX_ContactMaterialAsset* GetAsset() override;

	TWeakObjectPtr<UAGX_ContactMaterialInstance> Instance;
};
