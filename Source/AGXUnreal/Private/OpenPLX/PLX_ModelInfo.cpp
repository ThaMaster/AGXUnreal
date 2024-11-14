// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "OpenPLX/PLX_ModelInfo.h"


UPLX_ModelInfo* UPLX_ModelInfo::GetFrom(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return nullptr;

	return World->GetSubsystem<UPLX_ModelInfo>();
}

bool UPLX_ModelInfo::HasNative() const
{
	return Native.HasNative();
}

FPLXModelInfo* UPLX_ModelInfo::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &Native;
}

const FPLXModelInfo* UPLX_ModelInfo::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &Native;
}

void UPLX_ModelInfo::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPLX_ModelInfo::Deinitialize()
{
	Super::Deinitialize();
}
