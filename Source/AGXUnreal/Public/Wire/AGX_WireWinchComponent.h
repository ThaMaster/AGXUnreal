#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireWinch.h"
#include "AGX_NativeOwner.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_WireWinchComponent.generated.h"

UCLASS(ClassGroup = "AGX", BlueprintType, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WireWinchComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Wire Winch")
	FAGX_WireWinch WireWinch;

	/// @todo Rename these to something that doesn't mention the body explicitly, or at least makes
	/// it obvious that the body is optional and what happens when there is no body.
	/**
	 * Compute the location of this winch relative to the body attachment, if any. If there is no
	 * body attachment the the location will be computed relative to this Component instead.
	 *
	 * @return The location of the winch either relative to the attachment body or this Component.
	 */
	FVector ComputeBodyRelativeLocation();
	FRotator ComputeBodyRelativeRotation();

	//~ Begin IAGX_NativeOwner interface.
	/**
	 * @return True if a native AGX Dynamics representation has been created for this Wire
	 * Component.
	 */
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void AssignNative(uint64 NativeAddress) override;
	//~ End IAGX_NativeOwner interface.

	//~ Begin ActorComponent interface.
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End ActorComponent interface.

	// ~Begin UObject interface.
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.
};
