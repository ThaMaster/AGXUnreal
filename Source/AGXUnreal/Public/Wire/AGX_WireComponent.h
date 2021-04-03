#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_WireComponent.generated.h"

/**
 * A Wire is a lumped element structure with dynamic resolution, the wire will adapt the resolution
 * so that no unwanted vibrations will occur. The Wire is initialized from a set of routing nodes
 * that the user places but during runtime nodes may be created and removed as necessary. There are
 * different types of nodes and some nodes are persistent.
 */
UCLASS(ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class UAGX_WireComponent : public USceneComponent
{
public:
	GENERATED_BODY()

public:
	UAGX_WireComponent();

	//~ Begin ActorComponent Interface.

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnRegister() override;

	//~ End ActorComponent Interface.
};
