// Copyright 2022, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Materials/AGX_ContactMaterialInstance.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_ContactMaterialInstance* UAGX_ContactMaterialAsset::GetInstance()
{
	return Instance.Get();
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialAsset::GetOrCreateInstance(
	UAGX_ContactMaterialRegistrarComponent* Registrar)
{
	UAGX_ContactMaterialInstance* InstancePtr = Instance.Get();
	UWorld* PlayingWorld = Registrar->GetWorld();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ContactMaterialInstance::CreateFromAsset(Registrar, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
};

UAGX_ContactMaterialAsset* UAGX_ContactMaterialAsset::GetAsset()
{
	return this;
}

#if WITH_EDITOR
void UAGX_ContactMaterialAsset::PostInitProperties()
{
	Super::PostInitProperties();

	// All Contact Materials share the same FAGX_PropertyChangedDispatcher so DO NOT use any
	// captures or anything from the current 'this' in the Dispatcher callback. Only use the passed
	// parameter, which is a pointer to the object that was changed.
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	// These callbacks do not check the return value from GetInstance, it is the responsibility of
	// PostEditChangeProperty to only call FAGX_PropertyChangedDispatcher::Trigger when an instance
	// is available.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, ContactSolver),
		[](ThisClass* Asset) { Asset->GetInstance()->SetContactSolver(Asset->ContactSolver); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, ContactReduction),
		GET_MEMBER_NAME_CHECKED(FAGX_ContactMaterialReductionMode, Mode), [](ThisClass* Asset) {
			Asset->GetInstance()->SetContactReductionMode(Asset->ContactReduction.Mode);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, ContactReduction),
		GET_MEMBER_NAME_CHECKED(FAGX_ContactMaterialReductionMode, BinResolution),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetContactReductionBinResolution(
				Asset->ContactReduction.BinResolution);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, MechanicsApproach),
		GET_MEMBER_NAME_CHECKED(FAGX_ContactMaterialMechanicsApproach, bUseContactAreaApproach),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetUseContactAreaApproach(
				Asset->MechanicsApproach.bUseContactAreaApproach);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, MechanicsApproach),
		GET_MEMBER_NAME_CHECKED(FAGX_ContactMaterialMechanicsApproach, MinElasticRestLength),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetMinElasticRestLength(
				Asset->MechanicsApproach.MinElasticRestLength);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, MechanicsApproach),
		GET_MEMBER_NAME_CHECKED(FAGX_ContactMaterialMechanicsApproach, MaxElasticRestLength),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetMaxElasticRestLength(
				Asset->MechanicsApproach.MaxElasticRestLength);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, FrictionModel),
		[](ThisClass* Asset) { Asset->GetInstance()->SetFrictionModel(Asset->FrictionModel); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, NormalForceMagnitude),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetNormalForceMagnitude(Asset->NormalForceMagnitude);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, bScaleNormalForceWithDepth),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetScaleNormalForceWithDepth(Asset->bScaleNormalForceWithDepth);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, bSurfaceFrictionEnabled),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetSurfaceFrictionEnabled(Asset->bSurfaceFrictionEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, FrictionCoefficient),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetFrictionCoefficient(Asset->FrictionCoefficient);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, SecondaryFrictionCoefficient),
		[](ThisClass* Asset) {
			// When using InlineEditConditionToggle on the bool Unreal Editor triggers the value
			// callback, instead of the bool callback, when toggling the bool. So we don't know
			// which it is. Setting both.
			Asset->GetInstance()->SetUseSecondaryFrictionCoefficient(
				Asset->bUseSecondaryFrictionCoefficient);
			Asset->GetInstance()->SetSecondaryFrictionCoefficient(
				Asset->SecondaryFrictionCoefficient);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, bUseSecondaryFrictionCoefficient),
		[](ThisClass* Asset) {
			// This is currently (Unreal Engine 4.25) never called because Unreal Engine calls the
			// value callback when toggling the InlineEditConditionToggle. Leaving it here in case
			// that's changed in later versions.
			Asset->GetInstance()->SetUseSecondaryFrictionCoefficient(
				Asset->bUseSecondaryFrictionCoefficient);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, SurfaceViscosity), [](ThisClass* Asset) {
			Asset->GetInstance()->GetInstance()->SetSurfaceViscosity(Asset->SurfaceViscosity);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, SecondarySurfaceViscosity),
		[](ThisClass* Asset) {
			// When using InlineEditConditionToggle on the bool Unreal Editor triggers the value
			// callback, instead of the bool callback, when toggling the bool. So we don't know
			// which it is. Setting both.
			Asset->GetInstance()->SetUseSecondarySurfaceViscosity(
				Asset->bUseSecondarySurfaceViscosity);
			Asset->GetInstance()->SetSecondarySurfaceViscosity(Asset->SecondarySurfaceViscosity);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, bUseSecondarySurfaceViscosity),
		[](ThisClass* Asset) {
			// This is currently (Unreal Engine 4.25) never called because Unreal Engine calls the
			// value callback when toggling the InlineEditConditionToggle. Leaving it here in case
			// that's changed in later versions.
			Asset->GetInstance()->SetUseSecondarySurfaceViscosity(
				Asset->bUseSecondarySurfaceViscosity);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, PrimaryDirection), [](ThisClass* Asset) {
			Asset->GetInstance()->SetPrimaryDirection(Asset->PrimaryDirection);
		});

	// Here we would like to detect and handle changes to the Oriented Friction Reference Frame,
	// but that is currently not possible because to update the AGX Dynamics side we need to find
	// the Rigid Body Component, and to find the Rigid Body Component we need the Contact Material
	// Registrar that was used to create the Contact Material Instance. We currently don't store
	// that in the Contact Material Asset. See internal GitLab issue 707.
#if 0
	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, OrientedFrictionReferenceFrameComponent),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetOrientedFrictionReferenceFrameComponent(
				Asset->OrientedFrictionReferenceFrameComponent, Registrar);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, OrientedFrictionReferenceFrameActor),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetOrientedFrictionReferenceFrameActor(
				Asset->OrientedFrictionReferenceFrameActor, Registrar);
		});
#endif

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, Restitution),
		[](ThisClass* Asset) { Asset->GetInstance()->SetRestitution(Asset->Restitution); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, SpookDamping),
		[](ThisClass* Asset) { Asset->GetInstance()->SetSpookDamping(Asset->SpookDamping); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, YoungsModulus),
		[](ThisClass* Asset) { Asset->GetInstance()->SetYoungsModulus(Asset->YoungsModulus); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, AdhesiveForce),
		[](ThisClass* Asset) { Asset->GetInstance()->SetAdhesiveForce(Asset->AdhesiveForce); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, AdhesiveOverlap),
		[](ThisClass* Asset) { Asset->GetInstance()->SetAdhesiveOverlap(Asset->AdhesiveOverlap); });
}

void UAGX_ContactMaterialAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// UAGX_ContactMaterialAsset is not a Component and will not be destroyed and recreated
	// during RerunConstructionScript. It is therefore safe to call the base class
	// implementation immediately.
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Instance == nullptr)
	{
		// Nothing to do if there is no game/world instance to synchronize with.
		return;
	}

	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent);
}

#endif
