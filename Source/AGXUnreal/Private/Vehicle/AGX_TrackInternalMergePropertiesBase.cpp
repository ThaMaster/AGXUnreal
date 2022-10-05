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

UAGX_TrackInternalMergePropertiesBase::UAGX_TrackInternalMergePropertiesBase()
	: bMergeEnabled(false)
	, NumNodesPerMergeSegment(3)
	, ContactReduction(EAGX_MergedTrackNodeContactReduction::Minimal)
	, bLockToReachMergeConditionEnabled(true)
	, LockToReachMergeConditionCompliance(1.0E-11)
	, LockToReachMergeConditionDamping(3.0 / 60.0) // \todo Verify default value in AGX Dynamics!
	, MaxAngleMergeCondition(
		  FMath::RadiansToDegrees(1.0E-5)) // \todo Verify default value in AGX Dynamics!
{
	// See agxVehicle::TrackInternalMergeProperties for default values
}

UAGX_TrackInternalMergePropertiesBase::~UAGX_TrackInternalMergePropertiesBase()
{
}

#define COPY_PROPERTY(Source, Name) \
	{                               \
		Name = Source->Name;        \
	}

void UAGX_TrackInternalMergePropertiesBase::CopyFrom(
	const UAGX_TrackInternalMergePropertiesBase* Source)
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

EAGX_MergedTrackNodeContactReduction UAGX_TrackInternalMergePropertiesBase::ToContactReduction(
	uint8 ContactReduction)
{
	switch (ContactReduction)
	{
		case 0:
			return EAGX_MergedTrackNodeContactReduction::None;
		case 1:
			return EAGX_MergedTrackNodeContactReduction::Minimal;
		case 2:
			return EAGX_MergedTrackNodeContactReduction::Moderate;
		case 3:
			return EAGX_MergedTrackNodeContactReduction::Aggressive;
	}

	UE_LOG(
		LogAGX, Error, TEXT("Unknown ContactReduction '%d' passed to ContactReductionFrom()."),
		ContactReduction);
	return EAGX_MergedTrackNodeContactReduction::None;
}

void UAGX_TrackInternalMergePropertiesBase::CopyFrom(const FTrackBarrier& Barrier)
{
	bMergeEnabled = Barrier.InternalMergeProperties_GetEnableMerge();
	ContactReduction = ToContactReduction(Barrier.InternalMergeProperties_GetContactReduction());
	NumNodesPerMergeSegment = Barrier.InternalMergeProperties_GetNumNodesPerMergeSegment();
	bLockToReachMergeConditionEnabled =
		Barrier.InternalMergeProperties_GetEnableLockToReachMergeCondition();
	LockToReachMergeConditionCompliance =
		Barrier.InternalMergeProperties_GetLockToReachMergeConditionCompliance();
	LockToReachMergeConditionDamping =
		Barrier.InternalMergeProperties_GetLockToReachMergeConditionDamping();
	MaxAngleMergeCondition = Barrier.InternalMergeProperties_GetMaxAngleMergeCondition();
}

#if WITH_EDITOR

void UAGX_TrackInternalMergePropertiesBase::InitPropertyDispatcher()
{
	// All Track Properties share the same FAGX_PropertyChangedDispatcher so DO NOT use any captures
	// or anything from the current 'this' in the Dispatcher callback. Only use the passed
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
		GET_MEMBER_NAME_CHECKED(ThisClass, bMergeEnabled),
		[](ThisClass* Self)
		{
			Self->SetMergeEnabled(Self->bMergeEnabled);
			Self->GetInstance()->SetMergeEnabled(Self->bMergeEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NumNodesPerMergeSegment), [](ThisClass* Self)
		{ Self->GetInstance()->SetNumNodesPerMergeSegment(Self->NumNodesPerMergeSegment); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, ContactReduction),
		[](ThisClass* Self) { Self->GetInstance()->SetContactReduction(Self->ContactReduction); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bLockToReachMergeConditionEnabled),
		[](ThisClass* Self)
		{
			Self->GetInstance()->SetLockToReachMergeConditionEnabled(
				Self->bLockToReachMergeConditionEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, LockToReachMergeConditionCompliance),
		[](ThisClass* Self)
		{
			Self->GetInstance()->SetLockToReachMergeConditionCompliance(
				Self->LockToReachMergeConditionCompliance);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, LockToReachMergeConditionDamping),
		[](ThisClass* Self)
		{
			Self->GetInstance()->SetLockToReachMergeConditionDamping(
				Self->LockToReachMergeConditionDamping);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, MaxAngleMergeCondition), [](ThisClass* Self)
		{ Self->GetInstance()->SetMaxAngleMergeCondition(Self->MaxAngleMergeCondition); });
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

void UAGX_TrackInternalMergePropertiesBase::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	// UAGX_TrackProperties is not a Component and will not be destroyed and recreated
	// during RerunConstructionScript. It is therefore safe to call the base class
	// implementation immediately.
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent);
}
#endif

void UAGX_TrackInternalMergePropertiesBase::SetMergeEnabled(bool bEnabled)
{
	bMergeEnabled = bEnabled;
}

bool UAGX_TrackInternalMergePropertiesBase::GetMergeEnabled() const
{
	return bMergeEnabled;
}

void UAGX_TrackInternalMergePropertiesBase::SetNumNodesPerMergeSegment(
	int InNumNodesPerMergeSegment)
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

int32 UAGX_TrackInternalMergePropertiesBase::GetNumNodesPerMergeSegment() const
{
	return NumNodesPerMergeSegment;
}

void UAGX_TrackInternalMergePropertiesBase::SetContactReduction(
	EAGX_MergedTrackNodeContactReduction InContactReduction)
{
	ContactReduction = InContactReduction;
}

EAGX_MergedTrackNodeContactReduction UAGX_TrackInternalMergePropertiesBase::GetContactReduction()
	const
{
	return ContactReduction;
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionEnabled(bool bEnabled)
{
	bLockToReachMergeConditionEnabled = bEnabled;
}

bool UAGX_TrackInternalMergePropertiesBase::GetLockToReachMergeConditionEnabled() const
{
	return bLockToReachMergeConditionEnabled;
}

// Compliance.

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

double UAGX_TrackInternalMergePropertiesBase::GetLockToReachMergeConditionCompliance() const
{
	return LockToReachMergeConditionCompliance;
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionCompliance_BP(
	float Compliance)
{
	SetLockToReachMergeConditionCompliance(static_cast<double>(Compliance));
}

float UAGX_TrackInternalMergePropertiesBase::GetLockToReachMergeConditionCompliance_BP() const
{
	return static_cast<float>(GetLockToReachMergeConditionCompliance());
}

// Damping.

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

double UAGX_TrackInternalMergePropertiesBase::GetLockToReachMergeConditionDamping() const
{
	return LockToReachMergeConditionDamping;
}

void UAGX_TrackInternalMergePropertiesBase::SetLockToReachMergeConditionDamping_BP(float Damping)
{
	SetLockToReachMergeConditionDamping(static_cast<double>(Damping));
}

float UAGX_TrackInternalMergePropertiesBase::GetLockToReachMergeConditionDamping_BP() const
{
	return static_cast<float>(GetLockToReachMergeConditionDamping());
}

// Max Angle.

void UAGX_TrackInternalMergePropertiesBase::SetMaxAngleMergeCondition(double MaxAngleToMerge)
{
	if (MaxAngleToMerge < 0.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Attempt to set negative max angle merge condition on '%s'. Value clamped to 0.0"),
			*GetFullName());
		MaxAngleToMerge = 0.0;
	}

	MaxAngleMergeCondition = MaxAngleToMerge;
}

double UAGX_TrackInternalMergePropertiesBase::GetMaxAngleMergeCondition() const
{
	return MaxAngleMergeCondition;
}

void UAGX_TrackInternalMergePropertiesBase::SetMaxAngleMergeCondition_BP(float MaxAngleToMerge)
{
	SetMaxAngleMergeCondition(static_cast<double>(MaxAngleToMerge));
}

float UAGX_TrackInternalMergePropertiesBase::GetMaxAngleMergeCondition_BP() const
{
	return static_cast<float>(GetMaxAngleMergeCondition());
}
