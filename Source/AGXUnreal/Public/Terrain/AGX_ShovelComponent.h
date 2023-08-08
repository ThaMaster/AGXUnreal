// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal include.s
#include <AGX_NativeOwner.h>
#include <Terrain/ShovelBarrier.h>

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AGX_ShovelComponent.generated.h"

UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ShovelComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	// Sets default values for this Component's properties
	UAGX_ShovelComponent();

protected:
	// ~Begin UActorComponent interface.
	virtual void BeginPlay() override;
	// ~End UActorComponent interface.

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~/End IAGX_NativeOwner interface.

private:
	FShovelBarrier NativeBarrier;
};
