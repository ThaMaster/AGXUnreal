// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "RigidBodyBarrier.h"

// Sets default values for this component's properties
UAGX_ShovelComponent::UAGX_ShovelComponent()
{
	// Keep ticking off until we have a reason to turn it on.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UAGX_ShovelComponent::BeginPlay()
{
	Super::BeginPlay();

	/// @todo Call createNative.
}

bool UAGX_ShovelComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_ShovelComponent::GetNativeAddress() const
{
	// NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_ShovelComponent::SetNativeAddress(uint64 NativeAddress)
{
	NativeBarrier.SetNativeAddress(NativeAddress);
	// NativeBarrier.DecrementRefCount();
}

FShovelBarrier* UAGX_ShovelComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Shovel '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		AllocateNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
	return GetNative();
}

FShovelBarrier* UAGX_ShovelComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

const FShovelBarrier* UAGX_ShovelComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_ShovelComponent::AllocateNative()
{
	check(!GIsReconstructingBlueprintInstances);
	check(!HasNative());

	/// @todo Implement the Properties required to initialize the Native.
	FRigidBodyBarrier Body;
	NativeBarrier.AllocateNative(Body, FTwoVectors(), FTwoVectors(), FVector());
	check(HasNative()); /// \todo Consider better error handling than 'check'.

	WritePropertiesToNative();

	// No need to add Shovel to Simulation. Shovels only needs to be added to the Terrain, and that
	// is handled by the Terrain itself.
}

bool UAGX_ShovelComponent::WritePropertiesToNative()
{
	if (!HasNative())
	{
		return false;
	}

#if 1
	UE_LOG(
		LogAGX, Error, TEXT("UAGX_ShovelComponent::WritePropertiesToNative not yet implemented."));
#else
	NativeBarrier.SetToothLength();
	NativeBarrier.SetMaximumPenetrationForce();
	NativeBarrier.SetMaximumToothRadius();
	NativeBarrier.SetNumberOfTeeth();
	NativeBarrier.SetMinimumToothRadius();
	NativeBarrier.SetPenetrationDepthThreshold();
	NativeBarrier.SetPenetrationForceScaling();
	NativeBarrier.SetAlwaysRemoveShovelContacts();
	NativeBarrier.SetNoMergeExtensionDistance();
	NativeBarrier.SetSecondarySeparationDeadloadLimit();
	NativeBarrier.SetVerticalBladeSoilMergeDistance();
	NativeBarrier.SetMinimumSubmergedContactLengthFraction();

	NativeBarrier.SetExcavationSettingsEnabled();
	NativeBarrier.SetExcavationSettingsEnableForceFeedback();
	NativeBarrier.SetExcavationSettingsEnableCreateDynamicMass();

	// Properties initialized by the AGX Dynamics Shovel constructor are not set here.
#endif

	return true;
}
