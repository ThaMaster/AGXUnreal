// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholdsInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_Simulation.h"
#include "AMOR/AGX_WireMergeSplitThresholdsAsset.h"

UAGX_WireMergeSplitThresholdsBase*
UAGX_WireMergeSplitThresholdsInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
}

void UAGX_WireMergeSplitThresholdsInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FWireMergeSplitThresholdsBarrier());

	NativeBarrier->AllocateNative();
	AGX_CHECK(HasNative());

	UpdateNativeProperties();
}

FWireMergeSplitThresholdsBarrier*
UAGX_WireMergeSplitThresholdsInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}

	return NativeBarrier.Get();
}

bool UAGX_WireMergeSplitThresholdsInstance::HasNative()
{
	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_WireMergeSplitThresholdsInstance*
UAGX_WireMergeSplitThresholdsInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_WireMergeSplitThresholdsAsset& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_WireMergeSplitThresholdsInstance>(
		Outer, UAGX_WireMergeSplitThresholdsInstance::StaticClass(), *InstanceName,
		RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

void UAGX_WireMergeSplitThresholdsInstance::CopyProperties(
	UAGX_WireMergeSplitThresholdsAsset& Source)
{
	// Todo: implement.
}

void UAGX_WireMergeSplitThresholdsInstance::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	// TODO: implement.
}