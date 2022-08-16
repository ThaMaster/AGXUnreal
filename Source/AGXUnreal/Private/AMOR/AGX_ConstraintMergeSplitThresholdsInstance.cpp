// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_Simulation.h"
#include "AMOR/AGX_ConstraintMergeSplitThresholdsAsset.h"

UAGX_ConstraintMergeSplitThresholdsBase*
UAGX_ConstraintMergeSplitThresholdsInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FConstraintMergeSplitThresholdsBarrier());

	NativeBarrier->AllocateNative();
	AGX_CHECK(HasNative());

	UpdateNativeProperties();
}

FConstraintMergeSplitThresholdsBarrier*
UAGX_ConstraintMergeSplitThresholdsInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}

	return NativeBarrier.Get();
}

bool UAGX_ConstraintMergeSplitThresholdsInstance::HasNative()
{
	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_ConstraintMergeSplitThresholdsInstance*
UAGX_ConstraintMergeSplitThresholdsInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholdsAsset& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ConstraintMergeSplitThresholdsInstance>(
		Outer, UAGX_ConstraintMergeSplitThresholdsInstance::StaticClass(), *InstanceName,
		RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::CopyProperties(
	UAGX_ConstraintMergeSplitThresholdsAsset& Source)
{
	// Todo: implement.
}

void UAGX_ConstraintMergeSplitThresholdsInstance::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	// TODO: implement.
}