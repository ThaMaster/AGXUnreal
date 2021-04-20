#include "AGX_NativeOwnerInstanceData.h"

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_LogCategory.h"

FAGX_NativeOwnerInstanceData::FAGX_NativeOwnerInstanceData(
	const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
	TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster)
	: FSceneComponentInstanceData(SourceComponent)
	, Downcaster(InDowncaster)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("FAGX_NativeOwnerInstanceData created from NativeOwner 0x%llx under Reconstruct flag "
			 "%d."),
		(void*) NativeOwner, GIsReconstructingBlueprintInstances);

	NativeAddress = NativeOwner->GetNativeAddress();

	UE_LOG(
		LogAGX, Warning, TEXT("FAGX_NativeOwnerInstanceData stored native address 0x%llx."),
		NativeAddress);
}

void FAGX_NativeOwnerInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	FSceneComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);

	UE_LOG(
		LogAGX, Warning,
		TEXT("FAGX_NativeOwnerInstanceData with stored native address 0x%llx applying to "
			 "component 0x%llx under Reconstruct flag %d."),
		NativeAddress, Component, GIsReconstructingBlueprintInstances);

	/// @todo Will this compile? IAGX_NativeOwner is not a subclass of UActorComponent.
	// IAGX_NativeOwner* NativeOwner = static_cast<IAGX_NativeOwner*>(Component);
	//IAGX_NativeOwner* NativeOwner = SourceNativeOwner->AsNativeOwner(Component);
	IAGX_NativeOwner* NativeOwner = Downcaster(Component);
	if (NativeOwner == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_NativeOwnerInstanceData::ApplyToComponent called on something not a "
				 "IAGX_NativeOwner. This is a bug. The created Component may malfunction."));
		return;
	}

	if (NativeOwner->GetNativeAddress() == NativeAddress)
	{
		// Unreal Engine calls Apply twice, so detect that and do nothing the second time.
		UE_LOG(LogAGX, Warning, TEXT("  NativeOwner already has the correct Native Address, doing nothing"));
		return;
	}

	NativeOwner->AssignNative(NativeAddress);
}

bool FAGX_NativeOwnerInstanceData::ContainsData() const
{
	UE_LOG(
		LogAGX, Warning, TEXT("FAGX_NativeOwnerInstanceData ContainsData for address 0x%llx."),
		NativeAddress);

	// Extend with more tests once we store more data.
	return Super::ContainsData() || HasNativeAddress();
}

void FAGX_NativeOwnerInstanceData::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);

	/// \todo Do we need to do something here? What about Constraints outside of this Blueprint that
	/// point here?
}

void FAGX_NativeOwnerInstanceData::FindAndReplaceInstances(
	const TMap<UObject*, UObject*>& OldToNewInstanceMap)
{
	Super::FindAndReplaceInstances(OldToNewInstanceMap);

	/// \todo Do we need to do something here? What about Constraints outside of this Blueprint that
	/// point here?
}

bool FAGX_NativeOwnerInstanceData::HasNativeAddress() const
{
	return NativeAddress != 0;
}
