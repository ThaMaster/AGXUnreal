// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsBase.h"

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxImpactSpeed_AsFloat(
	float InMaxImpactSpeed)
{
	SetMaxImpactSpeed(FAGX_Real(InMaxImpactSpeed));
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxImpactSpeed(
	FAGX_Real InMaxImpactSpeed)
{
	MaxImpactSpeed = InMaxImpactSpeed;
}

float UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxImpactSpeed_AsFloat() const
{
	return static_cast<float>(GetMaxImpactSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxImpactSpeed() const
{
	return MaxImpactSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxRelativeNormalSpeed_AsFloat(float InMaxRelativeNormalSpeed)
{
	SetMaxRelativeNormalSpeed(FAGX_Real(InMaxRelativeNormalSpeed));
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxRelativeNormalSpeed(FAGX_Real InMaxRelativeNormalSpeed)
{
	MaxRelativeNormalSpeed = InMaxRelativeNormalSpeed;
}

float UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxRelativeNormalSpeed_AsFloat() const
{
	return static_cast<float>(GetMaxRelativeNormalSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxRelativeNormalSpeed() const
{
	return MaxRelativeNormalSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxRelativeTangentSpeed_AsFloat(float InMaxRelativeTangentSpeed)
{
	SetMaxRelativeTangentSpeed(FAGX_Real(InMaxRelativeTangentSpeed));
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxRelativeTangentSpeed(FAGX_Real InMaxRelativeTangentSpeed)
{
	MaxRelativeTangentSpeed = InMaxRelativeTangentSpeed;
}

float UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxRelativeTangentSpeed_AsFloat() const
{
	return static_cast<float>(GetMaxRelativeTangentSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxRelativeTangentSpeed() const
{
	return MaxRelativeTangentSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxRollingSpeed_AsFloat(float InMaxRollingSpeed)
{
	SetMaxRollingSpeed(FAGX_Real(InMaxRollingSpeed));
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed)
{
	MaxRollingSpeed = InMaxRollingSpeed;
}

float UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxRollingSpeed_AsFloat() const
{
	return static_cast<float>(GetMaxRollingSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsBase::GetMaxRollingSpeed() const
{
	return MaxRollingSpeed;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetNormalAdhesion_AsFloat(float InNormalAdhesion)
{
	SetNormalAdhesion(FAGX_Real(InNormalAdhesion));
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetNormalAdhesion(FAGX_Real InNormalAdhesion)
{
	NormalAdhesion = InNormalAdhesion;
}

float UAGX_ShapeContactMergeSplitThresholdsBase::GetNormalAdhesion_AsFloat() const
{
	return static_cast<float>(GetNormalAdhesion());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsBase::GetNormalAdhesion() const
{
	return NormalAdhesion;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetTangentialAdhesion_AsFloat(float InTangentialAdhesion)
{
	SetTangentialAdhesion(FAGX_Real(InTangentialAdhesion));
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetTangentialAdhesion(FAGX_Real InTangentialAdhesion)
{
	TangentialAdhesion = InTangentialAdhesion;
}

float UAGX_ShapeContactMergeSplitThresholdsBase::GetTangentialAdhesion_AsFloat() const
{
	return static_cast<float>(GetTangentialAdhesion());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholdsBase::GetTangentialAdhesion() const
{
	return TangentialAdhesion;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetMaySplitInGravityField(
	bool bInMaySplitInGravityField)
{
	bMaySplitInGravityField = bInMaySplitInGravityField;
}

bool UAGX_ShapeContactMergeSplitThresholdsBase::GetMaySplitInGravityField() const
{
	return bMaySplitInGravityField;
}

void UAGX_ShapeContactMergeSplitThresholdsBase::SetSplitOnLogicalImpact(bool bInbSplitOnLogicalImpact)
{
	bSplitOnLogicalImpact = bInbSplitOnLogicalImpact;
}

bool UAGX_ShapeContactMergeSplitThresholdsBase::GetSplitOnLogicalImpact() const
{
	return bSplitOnLogicalImpact;
}