// Copyright 2022, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackInternalMergePropertiesBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Vehicle/AGX_TrackInternalMergePropertiesInstance.h"

// AGX Dynamics for Unreal Barrier includes.
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_TrackInternalMergePropertiesInstance* UAGX_TrackInternalMergePropertiesBase::GetOrCreateInstance(
	UWorld* PlayingWorld, UAGX_TrackInternalMergePropertiesBase*& Property)
{
	if (Property == nullptr || PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		return nullptr;
	}

	UAGX_TrackInternalMergePropertiesInstance* Instance = Property->GetOrCreateInstance(PlayingWorld);

	if (Instance != Property)
	{
		Property = Instance;
	}

	return Instance;
}

UAGX_TrackInternalMergePropertiesBase::UAGX_TrackInternalMergePropertiesBase()
	: bMergeEnabled(false)
	, NumNodesPerMergeSegment(3)
	, ContactReduction(EAGX_MergedTrackNodeContactReduction::Minimal)
	, bLockToReachMergeConditionEnabled(true)
	, LockToReachMergeConditionCompliance(1.0E-11)
	, LockToReachMergeConditionDamping(3.0 / 60.0) // \todo Verify default value in AGX Dynamics!
	, MaxAngleMergeCondition(FMath::RadiansToDegrees(1.0E-5)) // \todo Verify default value in AGX Dynamics!
{
	// See agxVehicle::TrackInternalMergeProperties for default values
}

UAGX_TrackInternalMergePropertiesBase::~UAGX_TrackInternalMergePropertiesBase()
{
}

#define COPY_PROPERTY(Source, Name) \
	{                                   \
		Name = Source->Name;            \
	}

void UAGX_TrackInternalMergePropertiesBase::CopyFrom(const UAGX_TrackInternalMergePropertiesBase* Source)
{
	if (Source)
	{
		/// \todo Is there a way to make this in a more implicit way? Easy to forget these when
		/// adding properties.

		COPY_PROPERTY(Source, bMergeEnabled);
		COPY_PROPERTY(Source, NumNodesPerMergeSegment);
		COPY_PROPERTY(Source, ContactReduction);
		COPY_PROPERTY(Source, bLockToReachMergeConditionEnabled);
		COPY_PROPERTY(Source, LockToReachMergeConditionCompliance);
		COPY_PROPERTY(Source, LockToReachMergeConditionDamping);
		COPY_PROPERTY(Source, MaxAngleMergeCondition);
	}
}

#undef COPY_PROPERTY

#if WITH_EDITOR

void UAGX_TrackInternalMergePropertiesBase::InitPropertyDispatcher()
{

	// All Track Properties share the same FAGX_PropertyChangedDispatcher so DO NOT use any captures or
	// anything from the current 'this' in the Dispatcher callback. Only use the passed parameter,
	// which is a pointer to the object that was changed.
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher = FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	// These callbacks do not check the return value from GetInstance, it is the responsibility of
	// PostEditChangeProperty to only call FAGX_PropertyChangedDispatcher::Trigger when an instance is
	// available.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bMergeEnabled),
		[](ThisClass* Self) {
			Self->GetInstance()->SetMergeEnabled(
				Self->bMergeEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NumNodesPerMergeSegment),
		[](ThisClass* Self) {
			Self->GetInstance()->SetNumNodesPerMergeSegment(
				Self->NumNodesPerMergeSegment);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, ContactReduction),
		[](ThisClass* Self) {
			Self->GetInstance()->SetContactReduction(
				Self->ContactReduction);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bLockToReachMergeConditionEnabled),
		[](ThisClass* Self) {
			Self->GetInstance()->SetLockToReachMergeConditionEnabled(
				Self->bLockToReachMergeConditionEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, LockToReachMergeConditionCompliance),
		[](ThisClass* Self) {
			Self->GetInstance()->SetLockToReachMergeConditionCompliance(
				Self->LockToReachMergeConditionCompliance);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, LockToReachMergeConditionDamping),
		[](ThisClass* Self) {
			Self->GetInstance()->SetLockToReachMergeConditionDamping(
				Self->LockToReachMergeConditionDamping);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, MaxAngleMergeCondition),
		[](ThisClass* Self) {
			Self->GetInstance()->SetMaxAngleMergeCondition(
				Self->MaxAngleMergeCondition);
		});
}

#endif

void UAGX_TrackInternalMergePropertiesBase::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}


#if WITH_EDITOR

void UAGX_TrackInternalMergePropertiesBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UE_LOG(LogTemp, Log, TEXT("UAGX_TrackInternalMergePropertiesBase::PostEditChangeProperty"));

	// UAGX_TrackPropertiesAsset is not a Component and will not be destroyed and recreated
	// during RerunConstructionScript. It is therefore safe to call the base class
	// implementation immediately.
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GetInstance() == nullptr)
	{
		// Nothing to do if there is no game/world instance to synchronize with.
		return;
	}

	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent);
}
#endif

void UAGX_TrackInternalMergePropertiesBase::SetMergeEnabled(bool bEnabled)
{
	bMergeEnabled = bEnabled;
}

void UAGX_TrackInternalMergePropertiesBase::SetNumNodesPerMergeSegment(int InNumNodesPerMergeSegment)
{
	if (InNumNodesPerMergeSegment <= 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Attempt ot set negative or zero number of nodes per merge segment. Value clamped "
				 "to 1."));
		InNumNodesPerMergeSegment = 1;
	}

	NumNodesPerMergeSegment = InNumNodesPerMergeSegment;
}

void UAGX_TrackInternalMergePropertiesBase::SetContactReduction(EAGX_MergedTrackNodeContactReduction InContactReduction)
{
	ContactReduction = InContactReduction;
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionEnabled(bool bEnabled)
{
	bLockToReachMergeConditionEnabled = bEnabled;
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionCompliance_AsFloat(float Compliance)
{
	SetLockToReachMergeConditionCompliance(static_cast<double>(Compliance));
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionCompliance(
	double Compliance)
{
	if (Compliance < 0.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Attempt to set negative compliance on '%s'. Value clamped to 0.0."),
			*GetFullName());

		Compliance = 0.0;
	}

	LockToReachMergeConditionCompliance = Compliance;
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionDamping_AsFloat(float Damping)
{
	SetLockToReachMergeConditionDamping(static_cast<double>(Damping));
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionDamping(double Damping)
{
	if (Damping < 0.0)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Attempt to set negative damping on '%s'. Value clamped to 0.0."),
			*GetFullName());
		Damping = 0.0;
	}

	LockToReachMergeConditionDamping = Damping;
}

void UAGX_TrackInternalMergePropertiesBase::SetMaxAngleMergeCondition_AsFloat(float MaxAngleToMerge)
{
	SetMaxAngleMergeCondition(static_cast<double>(MaxAngleToMerge));
}

void UAGX_TrackInternalMergePropertiesBase::SetMaxAngleMergeCondition(double MaxAngleToMerge)
{
	MaxAngleMergeCondition = MaxAngleToMerge;
}
