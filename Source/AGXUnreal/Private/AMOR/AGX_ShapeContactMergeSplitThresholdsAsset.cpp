// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsInstance.h"


UAGX_ShapeContactMergeSplitThresholdsBase*
UAGX_ShapeContactMergeSplitThresholdsAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ShapeContactMergeSplitThresholdsInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_ShapeContactMergeSplitThresholdsInstance::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR
void UAGX_ShapeContactMergeSplitThresholdsAsset::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, MaxImpactSpeed),
		[](ThisClass* This) { This->SetMaxImpactSpeed(This->MaxImpactSpeed); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, MaxRelativeNormalSpeed),
		[](ThisClass* This) { This->SetMaxRelativeNormalSpeed(This->MaxRelativeNormalSpeed); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, MaxRelativeTangentSpeed),
		[](ThisClass* This) { This->SetMaxRelativeTangentSpeed(This->MaxRelativeTangentSpeed); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, MaxRollingSpeed),
		[](ThisClass* This) { This->SetMaxRollingSpeed(This->MaxRollingSpeed); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, NormalAdhesion),
		[](ThisClass* This) { This->SetNormalAdhesion(This->NormalAdhesion); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, TangentialAdhesion),
		[](ThisClass* This) { This->SetTangentialAdhesion(This->TangentialAdhesion); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, bMaySplitInGravityField),
		[](ThisClass* This) { This->SetMaySplitInGravityField(This->bMaySplitInGravityField); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholdsBase, bSplitOnLogicalImpact),
		[](ThisClass* This) { This->SetSplitOnLogicalImpact(This->bSplitOnLogicalImpact); });
}
#endif

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetMaxImpactSpeed(FAGX_Real InMaxImpactSpeed)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxImpactSpeed(InMaxImpactSpeed);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxImpactSpeed = InMaxImpactSpeed;
	}
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsAsset::GetMaxImpactSpeed() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxImpactSpeed();
	}

	return MaxImpactSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetMaxRelativeNormalSpeed(FAGX_Real InMaxRelativeNormalSpeed)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxRelativeNormalSpeed(InMaxRelativeNormalSpeed);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxRelativeNormalSpeed = InMaxRelativeNormalSpeed;
	}
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsAsset::GetMaxRelativeNormalSpeed() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxRelativeNormalSpeed();
	}

	return MaxRelativeNormalSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetMaxRelativeTangentSpeed(FAGX_Real InMaxRelativeTangentSpeed)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxRelativeTangentSpeed(InMaxRelativeTangentSpeed);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxRelativeTangentSpeed = InMaxRelativeTangentSpeed;
	}
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsAsset::GetMaxRelativeTangentSpeed() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxRelativeTangentSpeed();
	}

	return MaxRelativeTangentSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxRollingSpeed(InMaxRollingSpeed);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxRollingSpeed = InMaxRollingSpeed;
	}
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsAsset::GetMaxRollingSpeed() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxRollingSpeed();
	}

	return MaxRollingSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetNormalAdhesion(FAGX_Real InNormalAdhesion)
{
	if (Instance != nullptr)
	{
		Instance->SetNormalAdhesion(InNormalAdhesion);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		NormalAdhesion = InNormalAdhesion;
	}
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsAsset::GetNormalAdhesion() const
{
	if (Instance != nullptr)
	{
		return Instance->GetNormalAdhesion();
	}

	return NormalAdhesion;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetTangentialAdhesion(FAGX_Real InTangentialAdhesion)
{
	if (Instance != nullptr)
	{
		Instance->SetTangentialAdhesion(InTangentialAdhesion);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		TangentialAdhesion = InTangentialAdhesion;
	}
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsAsset::GetTangentialAdhesion() const
{
	if (Instance != nullptr)
	{
		return Instance->GetTangentialAdhesion();
	}

	return TangentialAdhesion;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetMaySplitInGravityField(bool bInMaySplitInGravityField)
{
	if (Instance != nullptr)
	{
		Instance->SetMaySplitInGravityField(bInMaySplitInGravityField);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		bMaySplitInGravityField = bInMaySplitInGravityField;
	}
}

bool UAGX_ShapeContactMergeSplitThresholdsAsset::GetMaySplitInGravityField() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaySplitInGravityField();
	}

	return bMaySplitInGravityField;
}

void UAGX_ShapeContactMergeSplitThresholdsAsset::SetSplitOnLogicalImpact(
	bool bInSplitOnLogicalImpact)
{
	if (Instance != nullptr)
	{
		Instance->SetSplitOnLogicalImpact(bInSplitOnLogicalImpact);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		bSplitOnLogicalImpact = bInSplitOnLogicalImpact;
	}
}

bool UAGX_ShapeContactMergeSplitThresholdsAsset::GetSplitOnLogicalImpact() const
{
	if (Instance != nullptr)
	{
		return Instance->GetSplitOnLogicalImpact();
	}

	return bSplitOnLogicalImpact;
}