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
		NativeBarrier.SetPulledInWireLength(InPulledInLength);
	}
}

double FAGX_WireWinch::GetPulledInLength() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPulledInWireLength();
	}
	return PulledInLength;
}

void FAGX_WireWinch::EnableMotor()
{
	bMotorEnabled = true;
	if (HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
}

void FAGX_WireWinch::DisableMotor()
{
	bMotorEnabled = false;
	if (HasNative())
	{
		NativeBarrier.SetForceRange({0.0, 0.0});
	}
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
		NativeBarrier.SetSpeed(InTargetSpeed);
	}
}

double FAGX_WireWinch::GetTargetSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier.GetSpeed();
	}
	return TargetSpeed;
}

void FAGX_WireWinch::SetMotorForceRange(const FAGX_DoubleInterval& InForceRange)
{
	MotorForceRange = InForceRange;
	if (bMotorEnabled && HasNative())
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
	if (bMotorEnabled)
	{
		return MotorForceRange;
	}
	else
	{
		return {0.0, 0.0};
	}
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
	bBrakeEnabled = true;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
}

void FAGX_WireWinch::DisableBrake()
{
	bBrakeEnabled = false;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange({0.0, 0.0});
	}
}

void FAGX_WireWinch::SetBrakeEnabled(bool bEnable)
{
	if (bEnable)
	{
		EnableBrake();
	}
	else
	{
		DisableBrake();
	}
}

bool FAGX_WireWinch::IsBrakeEnabled() const
{
	return bBrakeEnabled;
}

void FAGX_WireWinch::SetBrakeForceRange(const FAGX_DoubleInterval& InForceRange)
{
	BrakeForceRange = InForceRange;
	if (bBrakeEnabled && HasNative())
	{
		NativeBarrier.SetBrakeForceRange(InForceRange);
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

void FAGX_WireWinch::EnableEmergencyBrake()
{
	SetEmergencyBrakeEnabled(true);
}

void FAGX_WireWinch::DisableEmergencyBrake()
{
	SetEmergencyBrakeEnabled(false);
}

void FAGX_WireWinch::SetEmergencyBrakeEnabled(bool bEnable)
{
	bEmergencyBrakeEnabled = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableForcedBrake(bEnable);
	}
}

bool FAGX_WireWinch::IsEmergencyBrakeEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnableForcedBrake();
	}
	return bEmergencyBrakeEnabled;
}

double FAGX_WireWinch::GetCurrentSpeed() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentSpeed();
}

double FAGX_WireWinch::GetCurrentMotorForce() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentForce();
}

double FAGX_WireWinch::GetCurrentBrakeForce() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentBrakeForce();
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
		Body, LocationSim, RotationSim.RotateVector(FVector::ForwardVector), PulledInLength);
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
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_WireWinch: Cannot read properties from native: Dont' have a native."));
		return;
	}
	NativeBarrier.SetPulledInWireLength(PulledInLength);
	NativeBarrier.SetAutoFeed(bAutoFeed);
	NativeBarrier.SetSpeed(TargetSpeed);
	if (bMotorEnabled)
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
	else
	{
		NativeBarrier.SetForceRange({0.0, 0.0});
	}
	if (bBrakeEnabled)
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
	else
	{
		NativeBarrier.SetBrakeForceRange({0.0, 0.0});
	}

	NativeBarrier.SetEnableForcedBrake(bEmergencyBrakeEnabled);
}

void FAGX_WireWinch::ReadPropertiesFromNative()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_WireWinch: Cannot read properties from native: Dont' have a native."));
		return;
	}

	PulledInLength = NativeBarrier.GetPulledInWireLength();
	bAutoFeed = NativeBarrier.GetAutoFeed();
	TargetSpeed = NativeBarrier.GetSpeed();
	if (bMotorEnabled)
	{
		MotorForceRange = NativeBarrier.GetForceRange();
	}
	if (bBrakeEnabled)
	{
		BrakeForceRange = NativeBarrier.GetBrakeForceRange();
	}
	bEmergencyBrakeEnabled = NativeBarrier.GetEnableForcedBrake();
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

/* Start of FAGX_WireWinch_BP. */

FAGX_WireWinch_BP::FAGX_WireWinch_BP(FAGX_WireWinch* InWinch)
	: Winch(InWinch)
{
}

bool FAGX_WireWinch_BP::IsValid() const
{
	return Winch != nullptr;
}

/* Start of Blueprint Function Library. */

void UAGX_WireWinch_FL::SetLocation(FAGX_WireWinch_BP Winch, const FVector& InLocation)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetLocation."));
		return;
	}
	Winch.Winch->Location = InLocation;
}

FVector UAGX_WireWinch_FL::GetLocation(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetLocation."));
		return FVector();
	}
	return Winch.Winch->Location;
}

void UAGX_WireWinch_FL::SetRotation(FAGX_WireWinch_BP Winch, const FRotator& InRotation)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetRotation."));
		return ;
	}
	Winch.Winch->Rotation = InRotation;
}

FRotator UAGX_WireWinch_FL::GetRotation(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to ."));
		return FRotator();
	}
	return Winch.Winch->Rotation;
}


bool UAGX_WireWinch_FL::SetBodyAttachment(FAGX_WireWinch_BP Winch, UAGX_RigidBodyComponent* Body)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetBodyAttachment."));
		return false;
	}
	return Winch.Winch->SetBodyAttachment(Body);
}
UAGX_RigidBodyComponent* UAGX_WireWinch_FL::GetBodyAttachment(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetBodyAttachment."));
		return nullptr;
	}
	return Winch.Winch->GetBodyAttachment();
}

void UAGX_WireWinch_FL::SetPulledInLength(FAGX_WireWinch_BP Winch, float InPulledInLength)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetPulledInLength."));
		return;
	}
	return Winch.Winch->SetPulledInLength(static_cast<double>(InPulledInLength));
}

float UAGX_WireWinch_FL::GetPulledInLength(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetPulledInLength."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetPulledInLength());
}

void UAGX_WireWinch_FL::SetMotorEnabled(FAGX_WireWinch_BP Winch, bool bMotorEnabled)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetMotorEnabled."));
		return;
	}
	return Winch.Winch->SetMotorEnabled(bMotorEnabled);
}

bool UAGX_WireWinch_FL::IsMotorEnabled(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to IsMotorEnabled."));
		return false;
	}
	return Winch.Winch->IsMotorEnabled();
}

void UAGX_WireWinch_FL::SetMotorForceRange(FAGX_WireWinch_BP Winch, float Min, float Max)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetMotorForceRange."));
		return;
	}
	return Winch.Winch->SetMotorForceRange(static_cast<double>(Min), static_cast<double>(Max));
}
float UAGX_WireWinch_FL::GetMotorForceRangeMin(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetMotorForceRangeMin."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetMotorForceRangeMin());
}

float UAGX_WireWinch_FL::GetMotorForceRangeMax(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetMotorForceRangeMax."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetMotorForceRangeMax());
}

void UAGX_WireWinch_FL::SetBrakeForceRange(FAGX_WireWinch_BP Winch, float Min, float Max)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetBrakeForceRange."));
		return;
	}
	return Winch.Winch->SetBrakeForceRange(static_cast<double>(Min), static_cast<double>(Max));
}

float UAGX_WireWinch_FL::GetBrakeForceRangeMin(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetBrakeForceRangeMin."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetBrakeForceRangeMin());
}

float UAGX_WireWinch_FL::GetBrakeForceRangeMax(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetBrakeForceRangeMax."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetBrakeForceRangeMax());
}

void UAGX_WireWinch_FL::SetBrakeEnabled(FAGX_WireWinch_BP Winch, bool bInBrakeEnabled)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetBrakeEnabled."));
		return;
	}
	return Winch.Winch->SetBrakeEnabled(bInBrakeEnabled);
}

bool UAGX_WireWinch_FL::IsBrakeEnabled(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to IsBrakeEnabled."));
		return false;
	}
	return Winch.Winch->IsBrakeEnabled();
}

void UAGX_WireWinch_FL::SetTargetSpeed(FAGX_WireWinch_BP Winch, float InTargetSpeed)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to SetTargetSpeed."));
		return;
	}
	return Winch.Winch->SetTargetSpeed(static_cast<double>(InTargetSpeed));
}

float UAGX_WireWinch_FL::GetTargetSpeed(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetTargetSpeed."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetTargetSpeed());
}

float UAGX_WireWinch_FL::GetCurrentSpeed(FAGX_WireWinch_BP Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid WireWinchApi passed to GetCurrentSpeed."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetCurrentSpeed());
}
