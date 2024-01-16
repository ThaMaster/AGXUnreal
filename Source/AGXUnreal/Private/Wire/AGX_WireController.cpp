// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireController.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Wire/AGX_WireComponent.h"

UAGX_WireController::UAGX_WireController()
{
	NativeBarrier.InitializeNative();
}

UAGX_WireController* UAGX_WireController::Get()
{
	return NewObject<UAGX_WireController>();
}

bool UAGX_WireController::IsWireWireActive() const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	return NativeBarrier.IsWireWireActive();
}

bool UAGX_WireController::SetCollisionsEnabled(
	UAGX_WireComponent* Wire1, UAGX_WireComponent* Wire2, bool bEnable)
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (!Wire1->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 1 passed to AGX_WireController::SetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	if (!Wire2->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 2 passed to AGX_WireController::SetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	return NativeBarrier.SetCollisionsEnabled(*Wire1->GetNative(), *Wire2->GetNative(), bEnable);
}

bool UAGX_WireController::GetCollisionsEnabled(
	const UAGX_WireComponent* Wire1, const UAGX_WireComponent* Wire2) const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (!Wire1->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 1 passed to AGX_WireController::GetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	if (!Wire2->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 2 passed to AGX_WireController::GetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	return NativeBarrier.GetCollisionsEnabled(*Wire1->GetNative(), *Wire2->GetNative());
}

bool UAGX_WireController::HasNative() const
{
	return NativeBarrier.HasNative();
}
