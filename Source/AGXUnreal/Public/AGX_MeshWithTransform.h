#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_MeshWithTransform.generated.h"

class UStaticMesh;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_MeshWithTransform
{
	GENERATED_USTRUCT_BODY()

	FAGX_MeshWithTransform() = default;
	FAGX_MeshWithTransform(const UStaticMesh* InMesh, const FTransform& InTransform);

	bool IsValid() const;

	TWeakObjectPtr<const UStaticMesh> Mesh;
	FTransform Transform;
};
