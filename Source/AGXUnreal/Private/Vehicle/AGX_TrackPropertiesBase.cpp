// Copyright 2022, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackPropertiesBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Vehicle/AGX_TrackPropertiesInstance.h"

// AGX Dynamics for Unreal Barrier includes.
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"


static const double DefaultTrackHingeCompliance = 1.0E-10;
static const double DefaultTrackHingeDamping = 2.0 / 60.0;


UAGX_TrackPropertiesInstance* UAGX_TrackPropertiesBase::GetOrCreateInstance(
	UWorld* PlayingWorld, UAGX_TrackPropertiesBase*& Property)
{
	if (Property == nullptr || PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		return nullptr;
	}

	UAGX_TrackPropertiesInstance* Instance = Property->GetOrCreateInstance(PlayingWorld);

	if (Instance != Property)
	{
		Property = Instance;
	}

	return Instance;
}

UAGX_TrackPropertiesBase::UAGX_TrackPropertiesBase()
	:
	  // Compliance
	  HingeComplianceTranslational_X(DefaultTrackHingeCompliance)
	, HingeComplianceTranslational_Y(DefaultTrackHingeCompliance)
	, HingeComplianceTranslational_Z(DefaultTrackHingeCompliance)
	, HingeComplianceRotational_X(DefaultTrackHingeCompliance)
	, HingeComplianceRotational_Y(DefaultTrackHingeCompliance)
	  // Damping
	, HingeDampingTranslational_X(DefaultTrackHingeDamping)
	, HingeDampingTranslational_Y(DefaultTrackHingeDamping)
	, HingeDampingTranslational_Z(DefaultTrackHingeDamping)
	, HingeDampingRotational_X(DefaultTrackHingeDamping)
	, HingeDampingRotational_Y(DefaultTrackHingeDamping)
	  // Range
	, bHingeRangeEnabled(true)
	, HingeRange(-120.0, 20.0)
	  // Merge/Split
	, bOnInitializeMergeNodesToWheelsEnabled(false)
	, bOnInitializeTransformNodesToWheelsEnabled(true)
	, TransformNodesToWheelsOverlap(1.0E-3 * 100)
	, NodesToWheelsMergeThreshold(-0.1)
	, NodesToWheelsSplitThreshold(-0.05)
	, NumNodesIncludedInAverageDirection(3)
	  // Stabilizing
	, MinStabilizingHingeNormalForce(100.0)
	, StabilizingHingeFrictionParameter(1.0)

{
	// See agxVehicle::TrackProperties for default values
}

UAGX_TrackPropertiesBase::~UAGX_TrackPropertiesBase()
{
}

#define COPY_PROPERTY(Source, Name) \
	{                                   \
		Name = Source->Name;            \
	}

void UAGX_TrackPropertiesBase::CopyFrom(const UAGX_TrackPropertiesBase* Source)
{
	if (Source)
	{
		/// \todo Is there a way to make this in a more implicit way? Easy to forget these when
		/// adding properties.

		// Compliance
		COPY_PROPERTY(Source, HingeComplianceTranslational_X);
		COPY_PROPERTY(Source, HingeComplianceTranslational_Y);
		COPY_PROPERTY(Source, HingeComplianceTranslational_Z);
		COPY_PROPERTY(Source, HingeComplianceRotational_X);
		COPY_PROPERTY(Source, HingeComplianceRotational_Y);
		// Damping
		COPY_PROPERTY(Source, HingeDampingTranslational_X);
		COPY_PROPERTY(Source, HingeDampingTranslational_Y);
		COPY_PROPERTY(Source, HingeDampingTranslational_Z);
		COPY_PROPERTY(Source, HingeDampingRotational_X);
		COPY_PROPERTY(Source, HingeDampingRotational_Y);
		// Range
		COPY_PROPERTY(Source, bHingeRangeEnabled);
		COPY_PROPERTY(Source, HingeRange);
		// Merge/Split
		COPY_PROPERTY(Source, bOnInitializeMergeNodesToWheelsEnabled);
		COPY_PROPERTY(Source, bOnInitializeTransformNodesToWheelsEnabled);
		COPY_PROPERTY(Source, TransformNodesToWheelsOverlap);
		COPY_PROPERTY(Source, NodesToWheelsMergeThreshold);
		COPY_PROPERTY(Source, NodesToWheelsSplitThreshold);
		COPY_PROPERTY(Source, NumNodesIncludedInAverageDirection);
		// Stabilizing
		COPY_PROPERTY(Source, MinStabilizingHingeNormalForce);
		COPY_PROPERTY(Source, StabilizingHingeFrictionParameter);
	}
}

#if WITH_EDITOR

void UAGX_TrackPropertiesBase::InitPropertyDispatcher()
{
	// All Track Properties properties share the same FAGX_PropertyChangedDispatcher so DO NOT use any
	// captures or anything from the current 'this' in the Dispatcher callback. Only use the passed
	// parameter, which is a pointer to the object that was changed.
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher = FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	// These callbacks do not check the return value from GetInstance, it is the responsibility of
	// PostEditChangeProperty to only call FAGX_PropertyChangedDispatcher::Trigger when an instance is
	// available.


	// Hinge Compliance Translational.

	auto HingeComplianceTranslationDispatcherFunc = [](ThisClass* Self) {
		Self->GetInstance()->SetHingeComplianceTranslational(
			Self->HingeComplianceTranslational_X,
			Self->HingeComplianceTranslational_Y,
			Self->HingeComplianceTranslational_Z);
	};

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceTranslational_X),
		HingeComplianceTranslationDispatcherFunc);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceTranslational_Y),
		HingeComplianceTranslationDispatcherFunc);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceTranslational_Z),
		HingeComplianceTranslationDispatcherFunc);


	// Hinge Compliance Rotational.

	auto HingeComplianceRotationalDispatcherFunc = [](ThisClass* Self) {
		Self->GetInstance()->SetHingeComplianceRotational(
			Self->HingeComplianceRotational_X,
			Self->HingeComplianceRotational_Y);
	};

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceRotational_X),
		HingeComplianceRotationalDispatcherFunc);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceRotational_Y),
		HingeComplianceRotationalDispatcherFunc);


	// Hinge Damping Translational.

	auto HingeDampingTranslationDispatcherFunc = [](ThisClass* Self) {
		Self->GetInstance()->SetHingeDampingTranslational(
			Self->HingeDampingTranslational_X,
			Self->HingeDampingTranslational_Y,
			Self->HingeDampingTranslational_Z);
	};

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeDampingTranslational_X),
		HingeDampingTranslationDispatcherFunc);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeDampingTranslational_Y),
		HingeDampingTranslationDispatcherFunc);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeDampingTranslational_Z),
		HingeDampingTranslationDispatcherFunc);


	// Hinge Damping Rotational.

	auto HingeDampingRotationalDispatcherFunc = [](ThisClass* Self) {
		Self->GetInstance()->SetHingeDampingRotational(
			Self->HingeDampingRotational_X,
			Self->HingeDampingRotational_Y);
	};

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeDampingRotational_X),
		HingeDampingRotationalDispatcherFunc);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeDampingRotational_Y),
		HingeDampingRotationalDispatcherFunc);


	// Hinge Range.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bHingeRangeEnabled),
		[](ThisClass* Self) {
			Self->GetInstance()->SetHingeRangeEnabled(
				Self->bHingeRangeEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeRange),
		[](ThisClass* Self) {
			Self->GetInstance()->SetHingeRange(
				Self->HingeRange);
		});


	// Merge/Split Properties.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bOnInitializeMergeNodesToWheelsEnabled),
		[](ThisClass* Self) {
			Self->GetInstance()->SetOnInitializeMergeNodesToWheelsEnabled(
				Self->bOnInitializeMergeNodesToWheelsEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bOnInitializeTransformNodesToWheelsEnabled),
		[](ThisClass* Self) {
			Self->GetInstance()->SetOnInitializeTransformNodesToWheelsEnabled(
				Self->bOnInitializeTransformNodesToWheelsEnabled);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, TransformNodesToWheelsOverlap),
		[](ThisClass* Self) {
			Self->GetInstance()->SetTransformNodesToWheelsOverlap(
				Self->TransformNodesToWheelsOverlap);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NodesToWheelsMergeThreshold),
		[](ThisClass* Self) {
			Self->GetInstance()->SetNodesToWheelsMergeThreshold(
				Self->NodesToWheelsMergeThreshold);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NodesToWheelsSplitThreshold),
		[](ThisClass* Self) {
			Self->GetInstance()->SetNodesToWheelsSplitThreshold(
				Self->NodesToWheelsSplitThreshold);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NumNodesIncludedInAverageDirection),
		[](ThisClass* Self) {
			Self->GetInstance()->SetNumNodesIncludedInAverageDirection(
				Self->NumNodesIncludedInAverageDirection);
		});


	// Stabilizing Properties.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, MinStabilizingHingeNormalForce),
		[](ThisClass* Self) {
			Self->GetInstance()->SetMinStabilizingHingeNormalForce(
				Self->MinStabilizingHingeNormalForce);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, StabilizingHingeFrictionParameter),
		[](ThisClass* Self) {
			Self->GetInstance()->SetStabilizingHingeFrictionParameter(
				Self->StabilizingHingeFrictionParameter);
		});

}

#endif

void UAGX_TrackPropertiesBase::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}


#if WITH_EDITOR

void UAGX_TrackPropertiesBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// UAGX_TrackPropertiesAsset is not a Component and will not be destroyed and recreated
	// during RerunConstructionScript. It is therefore safe to call the base class
	// implementation immediately.
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GetInstance() == nullptr)
	{
		// Nothing to do if there is no game/world instance to synchronize with.
		return;
	}

	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent
		);
}
#endif

void UAGX_TrackPropertiesBase::SetHingeComplianceTranslational_AsFloat(float X, float Y, float Z)
{
	SetHingeComplianceTranslational(X, Y, Z);
}

void UAGX_TrackPropertiesBase::SetHingeComplianceTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z)
{
	HingeComplianceTranslational_X = X;
	HingeComplianceTranslational_Y = Y;
	HingeComplianceTranslational_Z = Z;
}

void UAGX_TrackPropertiesBase::SetHingeComplianceRotational_AsFloat(float X, float Y)
{
	SetHingeComplianceRotational(X, Y);
}

void UAGX_TrackPropertiesBase::SetHingeComplianceRotational(FAGX_Real X, FAGX_Real Y)
{
	HingeComplianceRotational_X = X;
	HingeComplianceRotational_Y = Y;
}

void UAGX_TrackPropertiesBase::SetHingeDampingTranslational_AsFloat(float X, float Y, float Z)
{
	SetHingeDampingTranslational(X, Y, Z);
}

void UAGX_TrackPropertiesBase::SetHingeDampingTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z)
{
	HingeDampingTranslational_X = X;
	HingeDampingTranslational_Y = Y;
	HingeDampingTranslational_Z = Z;
}

void UAGX_TrackPropertiesBase::SetHingeDampingRotational_AsFloat(float X, float Y)
{
	SetHingeDampingRotational(X, Y);
}

void UAGX_TrackPropertiesBase::SetHingeDampingRotational(FAGX_Real X, FAGX_Real Y)
{
	HingeDampingRotational_X = X;
	HingeDampingRotational_Y = Y;
}

void UAGX_TrackPropertiesBase::SetHingeRangeEnabled(bool bEnable)
{
	bHingeRangeEnabled = bEnable;
}

void UAGX_TrackPropertiesBase::SetHingeRangeMinMax(float Min, float Max)
{
	SetHingeRange(FAGX_RealInterval(Min, Max));
}

void UAGX_TrackPropertiesBase::SetHingeRange(const FAGX_RealInterval& Range)
{
	HingeRange = Range;
}

void UAGX_TrackPropertiesBase::SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable)
{
	bOnInitializeMergeNodesToWheelsEnabled = bEnable;
}

void UAGX_TrackPropertiesBase::SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable)
{
	bOnInitializeTransformNodesToWheelsEnabled = bEnable;
}

void UAGX_TrackPropertiesBase::SetTransformNodesToWheelsOverlap_AsFloat(float Overlap)
{
	SetTransformNodesToWheelsOverlap(Overlap);
}

void UAGX_TrackPropertiesBase::SetTransformNodesToWheelsOverlap(FAGX_Real Overlap)
{
	TransformNodesToWheelsOverlap = Overlap;
}

void UAGX_TrackPropertiesBase::SetNodesToWheelsMergeThreshold_AsFloat(float MergeThreshold)
{
	SetNodesToWheelsMergeThreshold(MergeThreshold);
}

void UAGX_TrackPropertiesBase::SetNodesToWheelsMergeThreshold(FAGX_Real MergeThreshold)
{
	NodesToWheelsMergeThreshold = MergeThreshold;
}

void UAGX_TrackPropertiesBase::SetNodesToWheelsSplitThreshold_AsFloat(float SplitThreshold)
{
	SetNodesToWheelsSplitThreshold(SplitThreshold);
}

void UAGX_TrackPropertiesBase::SetNodesToWheelsSplitThreshold(FAGX_Real SplitThreshold)
{
	NodesToWheelsSplitThreshold = SplitThreshold;
}

void UAGX_TrackPropertiesBase::SetNumNodesIncludedInAverageDirection(int NumIncludedNodes)
{
	NumNodesIncludedInAverageDirection = std::max(0, NumIncludedNodes);
}

void UAGX_TrackPropertiesBase::SetMinStabilizingHingeNormalForce_AsFloat(float MinNormalForce)
{
	SetMinStabilizingHingeNormalForce(MinNormalForce);
}

void UAGX_TrackPropertiesBase::SetMinStabilizingHingeNormalForce(FAGX_Real MinNormalForce)
{
	MinStabilizingHingeNormalForce = MinNormalForce;
}

void UAGX_TrackPropertiesBase::SetStabilizingHingeFrictionParameter_AsFloat(float FrictionParameter)
{
	SetStabilizingHingeFrictionParameter(FrictionParameter);
}

void UAGX_TrackPropertiesBase::SetStabilizingHingeFrictionParameter(FAGX_Real FrictionParameter)
{
	StabilizingHingeFrictionParameter = FrictionParameter;
}

#undef COPY_PROPERTY
