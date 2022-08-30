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

bool UAGX_ShapeContactMergeSplitThresholdsInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_ShapeContactMergeSplitThresholdsInstance*
UAGX_ShapeContactMergeSplitThresholdsInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholdsAsset& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ShapeContactMergeSplitThresholdsInstance>(
		Outer, UAGX_ShapeContactMergeSplitThresholdsInstance::StaticClass(), *InstanceName,
		RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::CopyProperties(
	UAGX_ShapeContactMergeSplitThresholdsAsset& Source)
{
	// Todo: implement.
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	// TODO: implement.
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetMaxImpactSpeed(FAGX_Real InMaxImpactSpeed)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxImpactSpeed(InMaxImpactSpeed);
	}

	MaxImpactSpeed = InMaxImpactSpeed;
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsInstance::GetMaxImpactSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxImpactSpeed();
	}

	return MaxImpactSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetMaxRelativeNormalSpeed(
	FAGX_Real InMaxRelativeNormalSpeed)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxRelativeNormalSpeed(InMaxRelativeNormalSpeed);
	}

	MaxRelativeNormalSpeed = InMaxRelativeNormalSpeed;
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsInstance::GetMaxRelativeNormalSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxRelativeNormalSpeed();
	}

	return MaxRelativeNormalSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetMaxRelativeTangentSpeed(
	FAGX_Real InMaxRelativeTangentSpeed)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxRelativeTangentSpeed(InMaxRelativeTangentSpeed);
	}

	MaxRelativeTangentSpeed = InMaxRelativeTangentSpeed;
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsInstance::GetMaxRelativeTangentSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxRelativeTangentSpeed();
	}

	return MaxRelativeTangentSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxRollingSpeed(InMaxRollingSpeed);
	}

	MaxRollingSpeed = InMaxRollingSpeed;
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsInstance::GetMaxRollingSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxRollingSpeed();
	}

	return MaxRollingSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetNormalAdhesion(FAGX_Real InNormalAdhesion)
{
	if (HasNative())
	{
		NativeBarrier->SetNormalAdhesion(InNormalAdhesion);
	}

	NormalAdhesion = InNormalAdhesion;
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsInstance::GetNormalAdhesion() const
{
	if (HasNative())
	{
		return NativeBarrier->GetNormalAdhesion();
	}

	return NormalAdhesion;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetTangentialAdhesion(
	FAGX_Real InTangentialAdhesion)
{
	if (HasNative())
	{
		NativeBarrier->SetTangentialAdhesion(InTangentialAdhesion);
	}

	TangentialAdhesion = InTangentialAdhesion;
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsInstance::GetTangentialAdhesion() const
{
	if (HasNative())
	{
		return NativeBarrier->GetTangentialAdhesion();
	}

	return TangentialAdhesion;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetMaySplitInGravityField(
	bool bInMaySplitInGravityField)
{
	if (HasNative())
	{
		NativeBarrier->SetMaySplitInGravityField(bInMaySplitInGravityField);
	}

	bMaySplitInGravityField = bInMaySplitInGravityField;
}

bool UAGX_ShapeContactMergeSplitThresholdsInstance::GetMaySplitInGravityField() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaySplitInGravityField();
	}

	return bMaySplitInGravityField;
}

void UAGX_ShapeContactMergeSplitThresholdsInstance::SetSplitOnLogicalImpact(
	bool bInSplitOnLogicalImpact)
{
	if (HasNative())
	{
		NativeBarrier->SetSplitOnLogicalImpact(bInSplitOnLogicalImpact);
	}

	bSplitOnLogicalImpact = bInSplitOnLogicalImpact;
}

bool UAGX_ShapeContactMergeSplitThresholdsInstance::GetSplitOnLogicalImpact() const
{
	if (HasNative())
	{
		return NativeBarrier->GetSplitOnLogicalImpact();
	}

	return bSplitOnLogicalImpact;
}