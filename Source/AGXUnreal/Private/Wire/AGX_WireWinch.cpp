#include "Wire/AGX_WireWinch.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

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

void FAGX_WireWinch::EnableMotor()
{
	SetMotorEnabled(true);
}

void FAGX_WireWinch::DisableMotor()
{
	SetMotorEnabled(false);
}

void FAGX_WireWinch::SetMotorEnabled(bool bInEnable)
{
	if (bMotorEnabled == bInEnable)
	{
		return;
	}

	bMotorEnabled = bInEnable;

	if (bInEnable)
	{
		SetMotorForceRange(CachedMotorForceRange);
		CachedMotorForceRange = {0.0, 0.0};
	}
	else
	{
		CachedMotorForceRange = GetMotorForceRange();
		SetMotorForceRange(0.0, 0.0);
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
	if (HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
}

void FAGX_WireWinch::SetMotorForceRange(double InMin, double InMax)
{
	SetMotorForceRange({InMin, InMax});
}

void FAGX_WireWinch::SetMotorForceRangeMin(double InForceRangeMin)
{
	MotorForceRange.Min = InForceRangeMin;
	if (HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
}

void FAGX_WireWinch::SetMotorForceRangeMax(double InForceRangeMax)
{
	MotorForceRange.Max = InForceRangeMax;
	if (HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
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

void FAGX_WireWinch::SetBrakeForceRangeMin(double InForceRangeMin)
{
	BrakeForceRange.Min = InForceRangeMin;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
}

void FAGX_WireWinch::SetBrakeForceRangeMax(double InForceRangeMax)
{
	BrakeForceRange.Max = InForceRangeMax;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
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
	if (HasNative())
	{
		return NativeBarrier.GetBrakeForceRange().Min;
	}
	return BrakeForceRange.Min;
}

double FAGX_WireWinch::GetBrakeForceRangeMax() const
{
	if (HasNative())
	{
		return NativeBarrier.GetBrakeForceRange().Max;
	}
	return BrakeForceRange.Max;
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

void FAGX_WireWinch::BeginPlay()
{
	UE_LOG(LogAGX, Error, TEXT("NOT YET IMPLEMENTED: FAGX_WireWinch::BeginPlay."));
}

#if 0
void FAGX_WireWinch::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}
#endif

void FAGX_WireWinch::EndPlay(const EEndPlayReason::Type Reason)
{
	UE_LOG(LogAGX, Error, TEXT("NOT YET IMPLEMENTED: FAGX_WireWinch::BeginPlay."));
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
UAGX_RigidBodyComponent* UAGX_WireWinch_FL::GetBodyAttachment(
	UPARAM(ref) const FAGX_WireWinch& Winch)
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
