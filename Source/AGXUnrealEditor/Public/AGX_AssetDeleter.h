

/// @todo Experimental code, do not merge to master.
#pragma message("Experimental code, do not merge to master.")


#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialInterface.h"

#include "AGX_AssetDeleter.generated.h"

UCLASS(ClassGroup="AGX", Meta=(BlueprintSpawnableComponent))
class AGXUNREALEDITOR_API UAGX_AssetDeleter : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_AssetDeleter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Asset Deleter")
	UMaterialInterface* ToDelete = nullptr;
};
