// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "AGX_RigidBodyComponent.h"
#include "CoreGlobals.h"

class FRigidBodyBarrier;

#define LOCTEXT_NAMESPACE "AGX_ShovelComponent"

// Sets default values for this component's properties
UAGX_ShovelComponent::UAGX_ShovelComponent()
{
	// Keep ticking off until we have a reason to turn it on.
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ShovelComponent::SwapEdgeDirections()
{
	std::swap(TopEdge.Start, TopEdge.End);
	std::swap(CuttingEdge.Start, CuttingEdge.End);
	if (HasNative())
	{
		UAGX_RigidBodyComponent* BodyComponent = RigidBody.GetRigidBody();
		if (BodyComponent == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Shovel '%s' in '%s' does not have a Rigid Body. Ignoring this Shovel."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return false;
		}

		FRigidBodyBarrier* BodyBarrier = BodyComponent->GetNative();
		if (BodyBarrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Shovel '%s' in '%s' has a Rigid Body, '%s' in '%s', that could not create its "
					"AGX Dynamics representation. Ignoring this Shovel."),
				*GetName(), *GetLabelSafe(GetOwner()), *BodyComponent->GetName(),
				*GetLabelSafe(BodyComponent->GetOwner()));
			return false;
		}

		const FTwoVectors TopEdgeInBody = TopEdge.GetLocationsRelativeTo(BodyComponent);
		const FTwoVectors CuttingEdgeInBody = CuttingEdge.GetLocationsRelativeTo(BodyComponent);
		NativeBarrier.SetTopEdge(TopEdgeInBody);
		NativeBarrier.SetCuttingEdge(CuttingEdgeInBody);
	}
	return true;
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
	AActor* Owner = GetTypedOuter<AActor>();
	RigidBody.OwningActor = Owner;
	TopEdge.Start.Parent.OwningActor = Owner;
	TopEdge.End.Parent.OwningActor = Owner;
	CuttingEdge.Start.Parent.OwningActor = Owner;
	CuttingEdge.End.Parent.OwningActor = Owner;
	CuttingDirection.Parent.OwningActor = Owner;
}

void UAGX_ShovelComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		AllocateNative();
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_ShovelComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
	return Super::GetComponentInstanceData();
}

bool UAGX_ShovelComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_ShovelComponent::GetNativeAddress() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetNativeAddress called on Shovel Component '%s' in '%s' that does not have an "
				 "AGX Dynamics native."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return 0;
	}
	NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_ShovelComponent::SetNativeAddress(uint64 NativeAddress)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SetNativeAddress called on Shovel Component '%s' in '%s' that does not have an "
				 "AGX Dynamics native."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}
	NativeBarrier.SetNativeAddress(NativeAddress);
	NativeBarrier.DecrementRefCount();
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
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("'%s' in '%s': Could not allocate AGX Dynamics Shovel in "
				 "UAGX_ShovelComponent::GetOrCreateNative, nullptr will be returned to caller."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}
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

	UAGX_RigidBodyComponent* BodyComponent = RigidBody.GetRigidBody();
	if (BodyComponent == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shovel '%s' in '%s' does not have a Rigid Body. Ignoring this Shovel."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FRigidBodyBarrier* BodyBarrier = BodyComponent->GetOrCreateNative();
	if (BodyBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shovel '%s' in '%s' has a Rigid Body, '%s' in '%s', that could not create its "
				 "AGX Dynamics representation. Ignoring this Shovel."),
			*GetName(), *GetLabelSafe(GetOwner()), *BodyComponent->GetName(),
			*GetLabelSafe(BodyComponent->GetOwner()));
		return;
	}

	/// @todo Implement the remaining Properties required to initialize the Native.
	const FTwoVectors TopEdgeInBody = TopEdge.GetLocationsRelativeTo(BodyComponent);
	const FTwoVectors CuttingEdgeInBody = CuttingEdge.GetLocationsRelativeTo(BodyComponent);
	const FRotator CuttingRotation = CuttingDirection.GetRotationRelativeTo(BodyComponent);
	const FVector CuttingDirectionInBody = CuttingRotation.RotateVector(FVector::ForwardVector);
	NativeBarrier.AllocateNative(
		*BodyBarrier, TopEdgeInBody, CuttingEdgeInBody, CuttingDirectionInBody);
	check(HasNative()); /// \todo Consider better error handling than 'check'.

	WritePropertiesToNative();

	// No need to add Shovel to Simulation. Shovels only needs to be added to the Terrain, and that
	// is handled by the Terrain itself.

	UE_LOG(
		LogAGX, Log, TEXT("Shovel '%s' in '%s' has allocated AGX Dynamics native."), *GetName(),
		*GetLabelSafe(GetOwner()));
}

bool UAGX_ShovelComponent::WritePropertiesToNative()
{
	if (!HasNative())
	{
		return false;
	}

	if (ShovelProperties == nullptr)
	{
		// No properties means use the defaults everywhere. It is not an error.
		return true;
	}
	NativeBarrier.SetToothLength(ShovelProperties->ToothLength);
	NativeBarrier.SetMaximumPenetrationForce(ShovelProperties->MaximumPenetrationForce);
	NativeBarrier.SetMaximumToothRadius(ShovelProperties->MaximumToothRadius);
	NativeBarrier.SetNumberOfTeeth(ShovelProperties->NumberOfTeeth);
	NativeBarrier.SetMinimumToothRadius(ShovelProperties->MinimumToothRadius);
	NativeBarrier.SetPenetrationDepthThreshold(ShovelProperties->PenetrationDepthThreshold);
	NativeBarrier.SetPenetrationForceScaling(ShovelProperties->PenetrationForceScaling);
	NativeBarrier.SetAlwaysRemoveShovelContacts(ShovelProperties->AlwaysRemoveShovelContacts);
	NativeBarrier.SetNoMergeExtensionDistance(ShovelProperties->NoMergeExtensionDistance);
	NativeBarrier.SetSecondarySeparationDeadloadLimit(
		ShovelProperties->SecondarySeparationDeadloadLimit);
	NativeBarrier.SetVerticalBladeSoilMergeDistance(
		ShovelProperties->VerticalBladeSoilMergeDistance);
	NativeBarrier.SetMinimumSubmergedContactLengthFraction(
		ShovelProperties->MinimumSubmergedContactLengthFraction);

	auto SetExcavationSettings =
		[this](EAGX_ExcavationMode Mode, const FAGX_ShovelExcavationSettings& Settings)
	{
		NativeBarrier.SetExcavationSettingsEnabled(Mode, Settings.bEnabled);
		NativeBarrier.SetExcavationSettingsEnableCreateDynamicMass(
			Mode, Settings.bEnableCreateDynamicMass);
		NativeBarrier.SetExcavationSettingsEnableForceFeedback(Mode, Settings.bEnableForceFeedback);
	};

	SetExcavationSettings(
		EAGX_ExcavationMode::Primary, ShovelProperties->PrimaryExcavationSettings);
	SetExcavationSettings(
		EAGX_ExcavationMode::DeformBack, ShovelProperties->DeformBackExcavationSettings);
	SetExcavationSettings(
		EAGX_ExcavationMode::DeformRight, ShovelProperties->DeformRightExcavationSettings);
	SetExcavationSettings(
		EAGX_ExcavationMode::DeformLeft, ShovelProperties->DeformLeftExcavationSettings);

	// Properties initialized by the AGX Dynamics Shovel constructor, i.e. body, edges, and cutting
	// direction, are not set here.

	return true;
}

#undef LOCTEXT_NAMESPACE
