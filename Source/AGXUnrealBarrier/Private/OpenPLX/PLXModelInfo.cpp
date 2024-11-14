// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLXModelInfo.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"


FPLXModelInfo::FPLXModelInfo()
	: Native(std::make_unique<FPLXModelData>())
{
}

FPLXModelInfo::~FPLXModelInfo()
{
}

bool FPLXModelInfo::HasNative() const
{
	return Native != nullptr;
}


