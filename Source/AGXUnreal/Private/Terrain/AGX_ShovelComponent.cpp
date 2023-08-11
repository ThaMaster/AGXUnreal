// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CoreGlobals.h"

class FRigidBodyBarrier;

#define LOCTEXT_NAMESPACE "AGX_ShovelComponent"

// Sets default values for this component's properties
UAGX_ShovelComponent::UAGX_ShovelComponent()
{
	// Keep ticking off until we have a reason to turn it on.
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_ShovelComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after InitProperties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	//
	// The intention is to provide by default a local scope that is the Actor outer that this
	// component is part of. If the OwningActor is set anywhere else, such as in the Details Panel,
	// then that "else" should overwrite this value shortly.
	//
	// We use GetTypedOuter because we worry that in some cases the Owner may not yet have been set
	// but there will always be an outer chain. This worry may be unfounded.
	RigidBody.OwningActor = GetTypedOuter<AActor>();
}

void UAGX_ShovelComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		AllocateNative();
	}
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

	FRigidBodyBarrier* Body = RigidBody.GetRigidBodyBarrier();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shovel '%s' in '%s' does not have a Rigid Body. Ignoring this Shovel."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	/// @todo Implement the remaining Properties required to initialize the Native.
	NativeBarrier.AllocateNative(*Body, FTwoVectors(), FTwoVectors(), FVector());
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

#undef LOCTEXT_NAMESPACE
