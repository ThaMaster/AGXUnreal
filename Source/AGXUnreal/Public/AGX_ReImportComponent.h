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


	// The reason why these Guid maps are stored in this Component is
	// that we cannot store an ImportGuid into Static Mesh Components as we do for any imported
	// AGXUnreal Components.

	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the guid
	// of the owning Trimesh.
	UPROPERTY(EditAnywhere, Category = "AGX Re-import Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningTrimesh;

	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the guid
	// of the owning RenderData. 
	UPROPERTY(EditAnywhere, Category = "AGX Re-import Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningRenderData;
};
