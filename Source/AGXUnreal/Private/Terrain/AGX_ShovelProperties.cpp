// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Terrain/AGX_ShovelComponent.h"

template <typename StorageT, typename ParameterT>
void SetAndPropagateShovelProperty(
	UAGX_ShovelProperties& Properties, TArray<TWeakObjectPtr<UAGX_ShovelComponent>>& Shovels,
	StorageT* Storage, ParameterT NewValue,
	void (UAGX_ShovelProperties::*ComponentSetter)(ParameterT),
	void (FShovelBarrier::*BarrierSetter)(ParameterT))
{
	/*
	This implementation should follow the pattern set by AGX_ASSET_SETTER_IMPL_INTERNAL, but
	adapted due to the fact that we don't have a single Barrier object but instead a collection
	of Shovel Components that own the Barriers.

	The general idea is that changes made to an instance should propagate to Barriers, and changes
	made to an asset should be applied on the instance instead, if there is one.
	*/
	if (Properties.IsInstance())
	{
		*Storage = NewValue;
		for (TWeakObjectPtr<UAGX_ShovelComponent>& Shovel : Shovels)
		{
			if (Shovel.IsValid() && Shovel->HasNative())
			{
				(Shovel->GetNative()->*BarrierSetter)(NewValue);
			}
		}
	}
	else
	{
		if (Properties.GetInstance() != nullptr)
		{
			(Properties.GetInstance()->*ComponentSetter)(NewValue);
		}
		else
		{
			*Storage = NewValue;
		}
	}
}

#define AGX_SHOVEL_SETTER_IMPL(PropertyName)             \
	SetAndPropagateShovelProperty(                       \
		*this, Shovels, &PropertyName, In##PropertyName, \
		&UAGX_ShovelProperties::Set##PropertyName, &FShovelBarrier::Set##PropertyName)

void UAGX_ShovelProperties::SetAlwaysRemoveShovelContacts(bool InAlwaysRemoveShovelContacts)
{
	// AGX_SHOVEL_SETTER_IMPL(AlwaysRemoveShovelContacts);
	SetAndPropagateShovelProperty(
		*this, Shovels, &AlwaysRemoveShovelContacts, InAlwaysRemoveShovelContacts,
		&UAGX_ShovelProperties::SetAlwaysRemoveShovelContacts,
		&FShovelBarrier::SetAlwaysRemoveShovelContacts);
}

void UAGX_ShovelProperties::SetEnableInnerShapeCreateDynamicMass(
	bool InEnableInnerShapeCreateDynamicMass)
{
	AGX_SHOVEL_SETTER_IMPL(EnableInnerShapeCreateDynamicMass);
}

void UAGX_ShovelProperties::SetEnableParticleForceFeedback(bool InEnableParticleForceFeedback)
{
	AGX_SHOVEL_SETTER_IMPL(EnableParticleForceFeedback);
}

void UAGX_ShovelProperties::SetEnableParticleFreeDeformers(bool InEnableParticleFreeDeformers)
{
	AGX_SHOVEL_SETTER_IMPL(EnableParticleFreeDeformers);
}

void UAGX_ShovelProperties::SetMinimumSubmergedContactLengthFraction(
	double InMinimumSubmergedContactLengthFraction)
{
	AGX_SHOVEL_SETTER_IMPL(MinimumSubmergedContactLengthFraction);
}

void UAGX_ShovelProperties::SetVerticalBladeSoilMergeDistance(
	double InVerticalBladeSoilMergeDistance)
{
	AGX_SHOVEL_SETTER_IMPL(VerticalBladeSoilMergeDistance);
}

void UAGX_ShovelProperties::SetNoMergeExtensionDistance(double InNoMergeExtensionDistance)
{
	AGX_SHOVEL_SETTER_IMPL(NoMergeExtensionDistance);
}

void UAGX_ShovelProperties::SetNumberOfTeeth(int32 InNumberOfTeeth)
{
	AGX_SHOVEL_SETTER_IMPL(NumberOfTeeth);
}

void UAGX_ShovelProperties::SetToothLength(double InToothLength)
{
	AGX_SHOVEL_SETTER_IMPL(ToothLength);
}

void UAGX_ShovelProperties::SetMaximumToothRadius(double InMaximumToothRadius)
{
	AGX_SHOVEL_SETTER_IMPL(MaximumToothRadius);
}

void UAGX_ShovelProperties::SetMinimumToothRadius(double InMinimumToothRadius)
{
	AGX_SHOVEL_SETTER_IMPL(MinimumToothRadius);
}

// Introduced with AGX Dynamics 2.37.
#if 0
void UAGX_ShovelProperties::SetParticleInclusionMultiplier(double InParticleInclusionMultiplier)
{
	AGX_SHOVEL_SETTER_IMPL(ParticleInclusionMultiplier);
}
#endif

void UAGX_ShovelProperties::SetPenetrationDepthThreshold(double InPenetrationDepthThreshold)
{
	AGX_SHOVEL_SETTER_IMPL(PenetrationDepthThreshold);
}

void UAGX_ShovelProperties::SetPenetrationForceScaling(double InPenetrationForceScaling)
{
	AGX_SHOVEL_SETTER_IMPL(PenetrationForceScaling);
}

void UAGX_ShovelProperties::SetMaximumPenetrationForce(double InMaximumPenetrationForce)
{
	AGX_SHOVEL_SETTER_IMPL(MaximumPenetrationForce);
}

void UAGX_ShovelProperties::SetSecondarySeparationDeadloadLimit(
	double InSecondarySeparationDeadloadLimit)
{
	AGX_SHOVEL_SETTER_IMPL(SecondarySeparationDeadloadLimit);
}

void UAGX_ShovelProperties::PostInitProperties()
{
	UObject::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShovelProperties::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

UAGX_ShovelProperties* UAGX_ShovelProperties::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		AGX_CHECK(PlayingWorld == GetWorld());
		if (PlayingWorld != GetWorld())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("UAGX_ShovelProperties::GetOrCreateInstance called on an instance in a "
					 "different world."));
			return nullptr;
		}
		return this;
	}
	if (Instance.IsValid())
	{
		AGX_CHECK(PlayingWorld == Instance->GetWorld());
		if (PlayingWorld != Instance->GetWorld())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("UAGX_ShovelProperties::GetOrCreateInstance called when we already have an "
					 "instance in a different world."));
			return nullptr;
		}
		return Instance.Get();
	}
	if (!PlayingWorld->IsGameWorld())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create Shovel Properties instance because the given world is not a "
				 "game world."));
		return nullptr;
	}

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	if (Outer == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create Shovel Properties instance because there is no AGX Simulation "
				 "in the world."));
		return nullptr;
	}

	const FString InstanceName = GetName() + "_Instance";
	UAGX_ShovelProperties* NewInstance =
		NewObject<UAGX_ShovelProperties>(Outer, *InstanceName, RF_Transient, this);
	NewInstance->Asset = this;
	this->Instance = NewInstance;
	return NewInstance;
}

bool UAGX_ShovelProperties::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	return Asset != nullptr;
}

UAGX_ShovelProperties* UAGX_ShovelProperties::GetInstance()
{
	return Instance.Get();
}

UAGX_ShovelProperties* UAGX_ShovelProperties::GetAsset()
{
	return Asset.Get();
}

void UAGX_ShovelProperties::CommitToAsset()
{
	// TODO Figure out how CommitToAsset should work.
#if 0
	if (IsInstance())
	{
		if (FShapeMaterialBarrier* Barrier = this->GetNative())
		{
			Asset->CopyFrom(*Barrier);
		}
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
#endif
}

namespace AGX_ShovelProperties_helpers
{
	/*
	 *  The intention is that shovels should always unregister themselves before being destroyed.
	 *  If we ever find a nullptr shovel in a Shovel Properties then that is a sign that we've
	 * either misunderstood something or that there is something we don't know. Either way,
	 * investigation required.
	 */
	bool CheckNoNullptr(const TArray<TWeakObjectPtr<UAGX_ShovelComponent>>& Shovels)
	{
		for (const TWeakObjectPtr<UAGX_ShovelComponent>& Shovel : Shovels)
		{
			if (Shovel.Get() == nullptr)
			{
				return false;
			}
		}
		return true;
	}
}

void UAGX_ShovelProperties::RegisterShovel(UAGX_ShovelComponent& Shovel)
{
	if (!IsInstance())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shovel Component %s in %s called Register Shovel on non-instance Shovel "
				 "Properties %s."),
			*Shovel.GetName(), *GetLabelSafe(Shovel.GetOwner()), *GetName());
		return;
	}
	AGX_CHECK(AGX_ShovelProperties_helpers::CheckNoNullptr(Shovels));
	Shovels.AddUnique(&Shovel);
}

void UAGX_ShovelProperties::UnregisterShovel(UAGX_ShovelComponent& Shovel)
{
	if (!IsInstance())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shovel Component %s in %s called Unregister Shovel on non-instance Shovel "
				 "Properties %s."),
			*Shovel.GetName(), *GetLabelSafe(Shovel.GetOwner()), *GetName());
		return;
	}
	AGX_CHECK(AGX_ShovelProperties_helpers::CheckNoNullptr(Shovels));
	Shovels.RemoveSwap(&Shovel);
}

void UAGX_ShovelProperties::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_ASSET_DEFAULT_DISPATCHER(AlwaysRemoveShovelContacts);
	AGX_ASSET_DEFAULT_DISPATCHER(EnableInnerShapeCreateDynamicMass);
	AGX_ASSET_DEFAULT_DISPATCHER(EnableParticleForceFeedback);
	AGX_ASSET_DEFAULT_DISPATCHER(EnableParticleFreeDeformers);
	AGX_ASSET_DEFAULT_DISPATCHER(MinimumSubmergedContactLengthFraction);
	AGX_ASSET_DEFAULT_DISPATCHER(VerticalBladeSoilMergeDistance);
	AGX_ASSET_DEFAULT_DISPATCHER(NoMergeExtensionDistance);
	AGX_ASSET_DEFAULT_DISPATCHER(NumberOfTeeth);
	AGX_ASSET_DEFAULT_DISPATCHER(ToothLength);
	AGX_ASSET_DEFAULT_DISPATCHER(MaximumToothRadius);
	AGX_ASSET_DEFAULT_DISPATCHER(MinimumToothRadius);
// Introduced with AGX Dynamics 2.37.
#if 0
	AGX_ASSET_DEFAULT_DISPATCHER(ParticleInclusionMultiplier);
#endif
	AGX_ASSET_DEFAULT_DISPATCHER(PenetrationDepthThreshold);
	AGX_ASSET_DEFAULT_DISPATCHER(PenetrationForceScaling);
	AGX_ASSET_DEFAULT_DISPATCHER(MaximumPenetrationForce);
	AGX_ASSET_DEFAULT_DISPATCHER(SecondarySeparationDeadloadLimit);
}
