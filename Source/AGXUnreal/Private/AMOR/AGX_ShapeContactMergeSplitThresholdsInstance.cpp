// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_Simulation.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsAsset.h"

UAGX_ShapeContactMergeSplitThresholdsBase*
UAGX_ShapeContactMergeSplitThresholdsInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FShapeContactMergeSplitThresholdsBarrier());

	NativeBarrier->AllocateNative();
	AGX_CHECK(HasNative());

	UpdateNativeProperties();
}

FShapeContactMergeSplitThresholdsBarrier*
UAGX_ShapeContactMergeSplitThresholdsInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}

	return NativeBarrier.Get();
}

bool UAGX_ShapeContactMergeSplitThresholdsInstance::HasNative()
{
	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_ShapeContactMergeSplitThresholdsInstance*
UAGX_ShapeContactMergeSplitThresholdsInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholdsAsset& Source)
{
	AGX_CHECK(Source);
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source->GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ShapeContactMergeSplitThresholdsInstance>(
		Outer, UAGX_ShapeContactMergeSplitThresholdsInstance::StaticClass(), *InstanceName,
		RF_Transient);

	NewInstance->CopyShapeMaterialProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	// TODO: implement.
}