#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_NativeOwnerInstanceData.generated.h"

class IAGX_NativeOwner;

/**
 * Component Instance Data for AGX Dynamics for Unreal subclasses of SceneComponent that owns an
 * AGX Dynamics native object. Will store the address of the native object in the transaction, and
 * call AssignNative on the new owner when the transaction data is applied to a new Component
 * instance.
 *
 * This Instance Data may ONLY be used by Components that inherit from USceneComponent AND implement
 * the IAGX_NativeOwner interface.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_NativeOwnerInstanceData : public FSceneComponentInstanceData
{
	GENERATED_BODY();

	FAGX_NativeOwnerInstanceData() = default;

	FAGX_NativeOwnerInstanceData(
		const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
		TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster);

	virtual ~FAGX_NativeOwnerInstanceData() = default;

	//~ Begin FComponentInstanceData interface.
	virtual void ApplyToComponent(
		UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;

	virtual bool ContainsData() const override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	virtual void FindAndReplaceInstances(
		const TMap<UObject*, UObject*>& OldToNewInstanceMap) override;
	//~ End FComponentInstanceData interface.

	bool HasNativeAddress() const;

private:
	UPROPERTY()
	uint64 NativeAddress;

	// const IAGX_NativeOwner* SourceNativeOwner = nullptr;
	TFunction<IAGX_NativeOwner*(UActorComponent*)> Downcaster;
};
