// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholdsBase.h"

void UAGX_WireMergeSplitThresholdsBase::SetForcePropagationDecayScale_AsFloat(
	float InForcePropagationDecayScale)
{
	SetForcePropagationDecayScale(FAGX_Real(InForcePropagationDecayScale));
}

void UAGX_WireMergeSplitThresholdsBase::SetForcePropagationDecayScale(
	FAGX_Real InForcePropagationDecayScale)
{
	ForcePropagationDecayScale = InForcePropagationDecayScale;
}

float UAGX_WireMergeSplitThresholdsBase::GetForcePropagationDecayScale_AsFloat() const
{
	return static_cast<float>(GetForcePropagationDecayScale());
}

FAGX_Real UAGX_WireMergeSplitThresholdsBase::GetForcePropagationDecayScale() const
{
	return ForcePropagationDecayScale;
}

void UAGX_WireMergeSplitThresholdsBase::SetMergeTensionScale_AsFloat(
	float InMergeTensionScale)
{
	SetMergeTensionScale(FAGX_Real(InMergeTensionScale));
}

void UAGX_WireMergeSplitThresholdsBase::SetMergeTensionScale(
	FAGX_Real InMergeTensionScale)
{
	MergeTensionScale = InMergeTensionScale;
}

float UAGX_WireMergeSplitThresholdsBase::GetMergeTensionScale_AsFloat() const
{
	return static_cast<float>(GetMergeTensionScale());
}

FAGX_Real UAGX_WireMergeSplitThresholdsBase::GetMergeTensionScale() const
{
	return MergeTensionScale;
}