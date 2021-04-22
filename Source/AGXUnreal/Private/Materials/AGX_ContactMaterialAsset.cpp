#include "Materials/AGX_ContactMaterialAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_UpropertyDispatcher.h"
#include "Materials/AGX_ContactMaterialInstance.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_ContactMaterialInstance* UAGX_ContactMaterialAsset::GetInstance()
{
	return Instance.Get();
}

UAGX_ContactMaterialInstance* UAGX_ContactMaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ContactMaterialInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ContactMaterialInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
};

UAGX_ContactMaterialAsset* UAGX_ContactMaterialAsset::GetAsset()
{
	return this;
}

void UAGX_ContactMaterialAsset::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	// All Contact Materials share the same FAGX_UpropertyDispatcher so DO NOT use any captures or
	// anything from the current 'this' in the Dispatcher callback. Only use the passed parameter,
	// which is a pointer to the object that was changed.
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	// These callbacks do not check the instance. It is the responsibility of PostEditChangeProperty
	// to only call FAGX_UpropertyDispatcher::Trigger when an instance is available.

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
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, bSurfaceFrictionEnabled),
		[](ThisClass* Asset) {
			Asset->GetInstance()->SetSurfaceFrictionEnabled(Asset->bSurfaceFrictionEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, FrictionCoefficient),
		[](ThisClass* Asset) {
			Asset->GetInstance()->GetInstance()->SetFrictionCoefficient(Asset->FrictionCoefficient);
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
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, Restitution),
		[](ThisClass* Asset) { Asset->GetInstance()->SetRestitution(Asset->Restitution); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, Damping),
		[](ThisClass* Asset) { Asset->GetInstance()->SetDamping(Asset->Damping); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, YoungsModulus),
		[](ThisClass* Asset) { Asset->GetInstance()->SetYoungsModulus(Asset->YoungsModulus); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, AdhesiveForce),
		[](ThisClass* Asset) { Asset->GetInstance()->SetAdhesiveForce(Asset->AdhesiveForce); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialAsset, AdhesiveOverlap),
		[](ThisClass* Asset) { Asset->GetInstance()->SetAdhesiveOverlap(Asset->AdhesiveOverlap); });
#endif
}

#if WITH_EDITOR
void UAGX_ContactMaterialAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// UAGX_ContactMaterialAsset is not a Component and will not be destroyed and recreated during
	// RerunConstructionScript. It is therefore safe to call the base class implementation
	// immediately, to make sure we don't forget it on some future return branch.
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Instance == nullptr)
	{
		// Nothing to do if there is no game/world instance to synchronize with.
		return;
	}

	// The root property that contains the property that was changed.
	const FName Member = (PropertyChangedEvent.MemberProperty != NULL)
							 ? PropertyChangedEvent.MemberProperty->GetFName()
							 : NAME_None;

	// The leaf property that was changed. May be nested in a struct.
	const FName Property = (PropertyChangedEvent.Property != NULL)
							   ? PropertyChangedEvent.Property->GetFName()
							   : NAME_None;

	if (FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(Member, Property, this))
	{
		return;
	}

	// Any custom handling/updates not handled by the Property Dispatcher can be added here.
}
#endif
