// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ReImportComponent.generated.h"

/*
 * @todo add description
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ReImportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Re-import Info")
	FString FilePath;

	UPROPERTY(EditAnywhere, Category = "AGX Re-import Info")
	bool bIgnoreDisabledTrimeshes = false;

	// Key is the name of the imported Static Mesh Component and the value is the guid of the
	// owning Shape's native. The reason why this data is stored in this Component is that
	// we cannot store an ImportGuid into Static Mesh Components as we do for any imported AGXUnreal
	// Components.
	UPROPERTY(EditAnywhere, Category = "AGX Re-import Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningShape;
};
