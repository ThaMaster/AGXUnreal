// Copyright 2022, Algoryx Simulation AB.


#include "Vehicle/AGX_TrackPropertiesInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Vehicle/AGX_TrackPropertiesAsset.h"

// AGX Dynamics for Unreal Barrier includes.
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_TrackPropertiesInstance* UAGX_TrackPropertiesInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TrackPropertiesAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UAGX_TrackPropertiesInstance* NewInstance = NewObject<UAGX_TrackPropertiesInstance>(
		Outer, UAGX_TrackPropertiesInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyFrom(Source);
	NewInstance->SourceAsset = Source;

	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

UAGX_TrackPropertiesInstance::~UAGX_TrackPropertiesInstance()
{
}

UAGX_TrackPropertiesAsset* UAGX_TrackPropertiesInstance::GetAsset()
{
	return SourceAsset.Get();
}

FTrackPropertiesBarrier* UAGX_TrackPropertiesInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

FTrackPropertiesBarrier* UAGX_TrackPropertiesInstance::GetNative()
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

bool UAGX_TrackPropertiesInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

void UAGX_TrackPropertiesInstance::UpdateNativeProperties()
{
	if (HasNative())
	{
		// Compliance
		NativeBarrier->SetHingeCompliance(HingeComplianceTranslational_X, 0);
		NativeBarrier->SetHingeCompliance(HingeComplianceTranslational_Y, 1);
		NativeBarrier->SetHingeCompliance(HingeComplianceTranslational_Z, 2);
		NativeBarrier->SetHingeCompliance(HingeComplianceRotational_X, 3);
		NativeBarrier->SetHingeCompliance(HingeComplianceRotational_Y, 4);
		// Damping
		NativeBarrier->SetHingeDamping(HingeDampingTranslational_X, 0);
		NativeBarrier->SetHingeDamping(HingeDampingTranslational_Y, 1);
		NativeBarrier->SetHingeDamping(HingeDampingTranslational_Z, 2);
		NativeBarrier->SetHingeDamping(HingeDampingRotational_X, 3);
		NativeBarrier->SetHingeDamping(HingeDampingRotational_Y, 4);
		// Range
		NativeBarrier->SetEnableHingeRange(bHingeRangeEnabled);
		NativeBarrier->SetHingeRangeRange(HingeRange);
		// Merge/Split
		NativeBarrier->SetEnableOnInitializeMergeNodesToWheels(bOnInitializeMergeNodesToWheelsEnabled);
		NativeBarrier->SetEnableOnInitializeTransformNodesToWheels(bOnInitializeTransformNodesToWheelsEnabled);
		NativeBarrier->SetTransformNodesToWheelsOverlap(TransformNodesToWheelsOverlap);
		NativeBarrier->SetNodesToWheelsMergeThreshold(NodesToWheelsMergeThreshold);
		NativeBarrier->SetNodesToWheelsSplitThreshold(NodesToWheelsSplitThreshold);
		NativeBarrier->SetNumNodesIncludedInAverageDirection(NumNodesIncludedInAverageDirection);
		// Stabilizing
		NativeBarrier->SetMinStabilizingHingeNormalForce(MinStabilizingHingeNormalForce);
		NativeBarrier->SetStabilizingHingeFrictionParameter(StabilizingHingeFrictionParameter);
	}
}

UAGX_TrackPropertiesInstance* UAGX_TrackPropertiesInstance::GetInstance()
{
	return this;
}

UAGX_TrackPropertiesInstance* UAGX_TrackPropertiesInstance::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	return this;
};

void UAGX_TrackPropertiesInstance::CreateNative(UWorld* PlayingWorld)
{
	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_TrackPropertiesInstance::CreateNative is creating native TrackProperties \"%s\"."), *GetName());

	NativeBarrier.Reset(new FTrackPropertiesBarrier());
	NativeBarrier->AllocateNative();
	check(HasNative());

	UpdateNativeProperties();
}

void UAGX_TrackPropertiesInstance::SetHingeComplianceTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z)
{
	Super::SetHingeComplianceTranslational(X, Y, Z);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetHingeCompliance(this->HingeComplianceTranslational_X, 0);
	NativeBarrier->SetHingeCompliance(this->HingeComplianceTranslational_Y, 1);
	NativeBarrier->SetHingeCompliance(this->HingeComplianceTranslational_Z, 2);
}

void UAGX_TrackPropertiesInstance::SetHingeComplianceRotational(FAGX_Real X, FAGX_Real Y)
{
	Super::SetHingeComplianceRotational(X, Y);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetHingeCompliance(this->HingeComplianceRotational_X, 3);
	NativeBarrier->SetHingeCompliance(this->HingeComplianceRotational_Y, 4);
}

void UAGX_TrackPropertiesInstance::SetHingeDampingTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z)
{
	Super::SetHingeDampingTranslational(X, Y, Z);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetHingeDamping(this->HingeDampingTranslational_X, 0);
	NativeBarrier->SetHingeDamping(this->HingeDampingTranslational_Y, 1);
	NativeBarrier->SetHingeDamping(this->HingeDampingTranslational_Z, 2);
}

void UAGX_TrackPropertiesInstance::SetHingeDampingRotational(FAGX_Real X, FAGX_Real Y)
{
	Super::SetHingeDampingRotational(X, Y);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetHingeDamping(this->HingeComplianceRotational_X, 3);
	NativeBarrier->SetHingeDamping(this->HingeComplianceRotational_Y, 4);
}

void UAGX_TrackPropertiesInstance::SetHingeRangeEnabled(bool bEnable)
{
	Super::SetHingeRangeEnabled(bEnable);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetEnableHingeRange(this->bHingeRangeEnabled);
}

void UAGX_TrackPropertiesInstance::SetHingeRange(const FAGX_RealInterval& Range)
{
	Super::SetHingeRange(Range);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetHingeRangeRange(this->HingeRange);
}

void UAGX_TrackPropertiesInstance::SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable)
{
	Super::SetOnInitializeMergeNodesToWheelsEnabled(bEnable);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetEnableOnInitializeMergeNodesToWheels(
		this->bOnInitializeMergeNodesToWheelsEnabled);
}

void UAGX_TrackPropertiesInstance::SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable)
{
	Super::SetOnInitializeTransformNodesToWheelsEnabled(bEnable);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetEnableOnInitializeTransformNodesToWheels(
		this->bOnInitializeTransformNodesToWheelsEnabled);
}

void UAGX_TrackPropertiesInstance::SetTransformNodesToWheelsOverlap(FAGX_Real Overlap)
{
	Super::SetTransformNodesToWheelsOverlap(Overlap);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetTransformNodesToWheelsOverlap(this->TransformNodesToWheelsOverlap);
}

void UAGX_TrackPropertiesInstance::SetNodesToWheelsMergeThreshold(FAGX_Real MergeThreshold)
{
	Super::SetNodesToWheelsMergeThreshold(MergeThreshold);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetNodesToWheelsMergeThreshold(this->NodesToWheelsMergeThreshold);
}

void UAGX_TrackPropertiesInstance::SetNodesToWheelsSplitThreshold(FAGX_Real SplitThreshold)
{
	Super::SetNodesToWheelsSplitThreshold(SplitThreshold);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetNodesToWheelsSplitThreshold(this->NodesToWheelsSplitThreshold);
}

void UAGX_TrackPropertiesInstance::SetNumNodesIncludedInAverageDirection(int NumIncludedNodes)
{
	Super::SetNumNodesIncludedInAverageDirection(NumIncludedNodes);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetNumNodesIncludedInAverageDirection(this->NumNodesIncludedInAverageDirection);
}

void UAGX_TrackPropertiesInstance::SetMinStabilizingHingeNormalForce(FAGX_Real MinNormalForce)
{
	Super::SetMinStabilizingHingeNormalForce(MinNormalForce);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetMinStabilizingHingeNormalForce(this->MinStabilizingHingeNormalForce);
}


void UAGX_TrackPropertiesInstance::SetStabilizingHingeFrictionParameter(FAGX_Real FrictionParameter)
{
	Super::SetStabilizingHingeFrictionParameter(FrictionParameter);
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetStabilizingHingeFrictionParameter(this->StabilizingHingeFrictionParameter);
}
