#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_TwoBodyTireActor.generated.h"

class UAGX_TwoBodyTireComponent;
class UAGX_RigidBodyComponent;

/**
 * An Actor that has an AGX_TwoBodyTireComponent.
 */
UCLASS(
	ClassGroup = "AGX", Blueprintable,
	meta =
		(BlueprintSpawnableComponent,
		 ToolTip = "Actor with an AGX_TwoBodyTireComponent."))
class AGXUNREAL_API AAGX_TwoBodyTireActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_TwoBodyTireActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_RigidBodyComponent* TireRigidBodyComponent;

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_RigidBodyComponent* HubRigidBodyComponent;

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_TwoBodyTireComponent* TwoBodyTireComponent;
};
