// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_Edge.h"
#include "AGX_Frame.h"
#include "AGX_NativeOwner.h"
#include "AGX_RigidBodyReference.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/AGX_TerrainEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_ShovelComponent.generated.h"

class UAGX_ShovelProperties;

UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ShovelComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_ShovelComponent();

	/// The Rigid Body that is to be imbued with shovel behavior.
	UPROPERTY(EditAnywhere, Category = "AGX Shovel", Meta = (SkipUCSModifiedProperties))
	FAGX_RigidBodyReference RigidBody;

	/**
	 * Configuration properties for the Shovel. If not set then the AGX Dynamics defaults are
	 * used for all properties.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shovel")
	UAGX_ShovelProperties* ShovelProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shovel", Meta = (SkipUCSModifiedProperties))
	FAGX_Edge TopEdge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shovel", Meta = (SkipUCSModifiedProperties))
	FAGX_Edge CuttingEdge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shovel", Meta = (SkipUCSModifiedProperties))
	FAGX_Frame CuttingDirection;

	/**
	 * Get one of the frames that are used to define the edges and directions of the shovel.
	 */
	FAGX_Frame* GetFrame(EAGX_ShovelFrame Frame);

	bool SwapEdgeDirections();

	// ~Begin UActorComponent interface.
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	// ~End UActorComponent interface.

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~/End IAGX_NativeOwner interface.

	/// Get the native AGX Dynamics representation of this shovel. Create it if necessary.
	FShovelBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this shovel. May return nullptr.
	FShovelBarrier* GetNative();

	/// Return the native AGX Dynamics representation of this shovel. May return nullptr.
	const FShovelBarrier* GetNative() const;

	/// Write all Component Properties to the Native. This is rarely needed.
	bool WritePropertiesToNative();

private:
	// Create the native AGX Dynamics object.
	void AllocateNative();

private:
	FShovelBarrier NativeBarrier;
};
