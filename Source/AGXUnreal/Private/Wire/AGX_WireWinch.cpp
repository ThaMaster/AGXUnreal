#include "Wire/AGX_WireWinch.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "CoreGlobals.h"

bool FAGX_WireWinch::SetBodyAttachment(UAGX_RigidBodyComponent* Body)
{
	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("SetBodyAttachment called on Wire Winch that have already been initialized. "
				 "Cannot move initialized winches between bodies."));
		return false;
	}
	if (Body == nullptr)
	{
		BodyAttachment.OwningActor = nullptr;
		BodyAttachment.BodyName = NAME_None;
		return true;
	}

	BodyAttachment.OwningActor = Body->GetOwner();
	BodyAttachment.BodyName = Body->GetFName();
	return true;
}

UAGX_RigidBodyComponent* FAGX_WireWinch::GetBodyAttachment() const
{
	return BodyAttachment.GetRigidBody();
}

void FAGX_WireWinch::SetPulledInLength(double InPulledInLength)
{
	PulledInLength = InPulledInLength;
	if (HasNative())
	{
		NativeBarrier.SetPulledInLength(InPulledInLength);
	}
}

double FAGX_WireWinch::GetPulledInLength() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPulledInLength();
	}
	return PulledInLength;
}

namespace AGX_WireWinch_helpers
{
	void SetMotorState(FAGX_WireWinch& Winch, bool bEnabled, const FAGX_DoubleInterval& ForceRange)
	{
		Winch.bMotorEnabled = bEnabled;
		Winch.MotorForceRange = ForceRange;
		if (Winch.HasNative())
		{
			Winch.NativeBarrier.SetForceRange(ForceRange);
		}
	}
};

void FAGX_WireWinch::EnableMotor()
{
	if (bMotorEnabled)
	{
		return;
	}
	AGX_WireWinch_helpers::SetMotorState(*this, true, CachedMotorForceRange);
}

void FAGX_WireWinch::DisableMotor()
{
	if (!bMotorEnabled)
	{
		return;
	}
	AGX_WireWinch_helpers::SetMotorState(*this, false, {0.0, 0.0});
}

void FAGX_WireWinch::SetMotorEnabled(bool bInEnable)
{
	if (bInEnable)
	{
		EnableMotor();
	}
	else
	{
		DisableMotor();
	}
}

bool FAGX_WireWinch::IsMotorEnabled() const
{
	return bMotorEnabled;
}

void FAGX_WireWinch::SetTargetSpeed(double InTargetSpeed)
{
	TargetSpeed = InTargetSpeed;
	if (HasNative())
	{
		NativeBarrier.SetTargetSpeed(InTargetSpeed);
	}
}

double FAGX_WireWinch::GetTargetSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier.GetTargetSpeed();
	}
	return TargetSpeed;
}

void FAGX_WireWinch::SetMotorForceRange(const FAGX_DoubleInterval& InForceRange)
{
	MotorForceRange = InForceRange;
	CachedMotorForceRange = InForceRange;
	if (HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
}

void FAGX_WireWinch::SetMotorForceRange(double InMin, double InMax)
{
	SetMotorForceRange({InMin, InMax});
}

void FAGX_WireWinch::SetMotorForceRangeMin(double InMin)
{
	SetMotorForceRange({InMin, MotorForceRange.Max});
}

void FAGX_WireWinch::SetMotorForceRangeMax(double InMax)
{
	SetMotorForceRange({MotorForceRange.Min, InMax});
}

FAGX_DoubleInterval FAGX_WireWinch::GetMotorForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier.GetForceRange();
	}
	return MotorForceRange;
}

double FAGX_WireWinch::GetMotorForceRangeMin() const
{
	return GetMotorForceRange().Min;
}

double FAGX_WireWinch::GetMotorForceRangeMax() const
{
	return GetMotorForceRange().Max;
}

void FAGX_WireWinch::EnableBrake()
{
	SetBrakeEnabled(true);
}

void FAGX_WireWinch::DisableBrake()
{
	SetBrakeEnabled(false);
}

void FAGX_WireWinch::SetBrakeEnabled(bool bEnable)
{
	bBrakeEnabled = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetBrakeEnabled(bBrakeEnabled);
	}
}

bool FAGX_WireWinch::IsBrakeEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.IsBrakeEnabled();
	}
	return bBrakeEnabled;
}

void FAGX_WireWinch::SetBrakeForceRange(const FAGX_DoubleInterval& InBrakeForceRange)
{
	BrakeForceRange = InBrakeForceRange;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
}

void FAGX_WireWinch::SetBrakeForceRange(double InMin, double InMax)
{
	SetBrakeForceRange({InMin, InMax});
}

void FAGX_WireWinch::SetBrakeForceRangeMin(double InMin)
{
	SetBrakeForceRange({InMin, BrakeForceRange.Max});
}

void FAGX_WireWinch::SetBrakeForceRangeMax(double InMax)
{
	SetBrakeForceRange({BrakeForceRange.Min, InMax});
}

FAGX_DoubleInterval FAGX_WireWinch::GetBrakeForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier.GetBrakeForceRange();
	}
	return BrakeForceRange;
}

double FAGX_WireWinch::GetBrakeForceRangeMin() const
{
	return GetBrakeForceRange().Min;
}

double FAGX_WireWinch::GetBrakeForceRangeMax() const
{
	return GetBrakeForceRange().Max;
}

double FAGX_WireWinch::GetCurrentSpeed() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentSpeed();
}

bool FAGX_WireWinch::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 FAGX_WireWinch::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	NativeBarrier.IncrementRefCount();
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

/// @todo Rename to SetNativeAddress.
void FAGX_WireWinch::AssignNative(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
	NativeBarrier.DecrementRefCount();
}

void FAGX_WireWinch::CreateNative()
{
	check(!GIsReconstructingBlueprintInstances);
	check(!HasNative());
	FRigidBodyBarrier* Body = [this]() -> FRigidBodyBarrier*
	{
		UAGX_RigidBodyComponent* Body = BodyAttachment.GetRigidBody();
		if (Body == nullptr)
		{
			return nullptr;
		}
		return Body->GetOrCreateNative();
	}();
	NativeBarrier.AllocateNative(
		Body, Location, Rotation.RotateVector(FVector::ForwardVector), PulledInLength);
	WritePropertiesToNative();
}

FWireWinchBarrier* FAGX_WireWinch::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FWireWinchBarrier* FAGX_WireWinch::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

FWireWinchBarrier* FAGX_WireWinch::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

void FAGX_WireWinch::WritePropertiesToNative()
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Error, TEXT(""));
		return;
	}
	NativeBarrier.SetPulledInLength(PulledInLength);
	NativeBarrier.SetAutoFeed(bAutoFeed);
	NativeBarrier.SetTargetSpeed(TargetSpeed);
	NativeBarrier.SetForceRange(MotorForceRange);
	NativeBarrier.SetBrakeEnabled(bBrakeEnabled);
	NativeBarrier.SetBrakeForceRange(BrakeForceRange);
}

FAGX_WireWinch::FAGX_WireWinch(const FAGX_WireWinch& Other)
	: FAGX_WireWinchSettings(Other)
{
}

FAGX_WireWinch& FAGX_WireWinch::operator=(const FAGX_WireWinch& Other)
{
	FAGX_WireWinchSettings::operator=(Other);
	return *this;
}

/* Start of Blueprint Function Library. */

bool UAGX_WireWinch_FL::SetBodyAttachment(
	UPARAM(ref) FAGX_WireWinch& Winch, UAGX_RigidBodyComponent* Body)
{
	return Winch.SetBodyAttachment(Body);
}
UAGX_RigidBodyComponent* UAGX_WireWinch_FL::GetBodyAttachment(UPARAM(ref)
																  const FAGX_WireWinch& Winch)
{
	return Winch.GetBodyAttachment();
}

void UAGX_WireWinch_FL::SetPulledInLength(UPARAM(ref) FAGX_WireWinch& Winch, float InPulledInLength)
{
	return Winch.SetPulledInLength(static_cast<double>(InPulledInLength));
}

float UAGX_WireWinch_FL::GetPulledInLength(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetPulledInLength());
}

void UAGX_WireWinch_FL::SetMotorEnabled(UPARAM(ref) FAGX_WireWinch& Winch, bool bMotorEnabled)
{
	return Winch.SetMotorEnabled(bMotorEnabled);
}

bool UAGX_WireWinch_FL::IsMotorEnabled(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return Winch.IsMotorEnabled();
}

void UAGX_WireWinch_FL::SetMotorForceRange(UPARAM(ref) FAGX_WireWinch& Winch, float Min, float Max)
{
	return Winch.SetMotorForceRange(static_cast<double>(Min), static_cast<double>(Max));
}
float UAGX_WireWinch_FL::GetMotorForceRangeMin(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetMotorForceRangeMin());
}

float UAGX_WireWinch_FL::GetMotorForceRangeMax(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetMotorForceRangeMax());
}

void UAGX_WireWinch_FL::SetBrakeForceRange(UPARAM(ref) FAGX_WireWinch& Winch, float Min, float Max)
{
	return Winch.SetBrakeForceRange(static_cast<double>(Min), static_cast<double>(Max));
}

float UAGX_WireWinch_FL::GetBrakeForceRangeMin(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetBrakeForceRangeMin());
}

float UAGX_WireWinch_FL::GetBrakeForceRangeMax(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetBrakeForceRangeMax());
}

void UAGX_WireWinch_FL::SetBrakeEnabled(UPARAM(ref) FAGX_WireWinch& Winch, bool bInBrakeEnabled)
{
	return Winch.SetBrakeEnabled(bInBrakeEnabled);
}

bool UAGX_WireWinch_FL::IsBrakeEnabled(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return Winch.IsBrakeEnabled();
}

void UAGX_WireWinch_FL::SetTargetSpeed(UPARAM(ref) FAGX_WireWinch& Winch, float InTargetSpeed)
{
	return Winch.SetTargetSpeed(static_cast<double>(InTargetSpeed));
}
float UAGX_WireWinch_FL::GetTargetSpeed(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetTargetSpeed());
}

float UAGX_WireWinch_FL::GetCurrentSpeed(UPARAM(ref) const FAGX_WireWinch& Winch)
{
	return static_cast<float>(Winch.GetCurrentSpeed());
}
