#include "Constraints/AGX_ConstraintController.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintController::FAGX_ConstraintController()
	// This 'false' here doesn't really make much sense, but we must have a
	// default constructor and the default constructor must provide some value.
	// Should never be called for an actual ConstraintController.
	: FAGX_ConstraintController(false)
{
}

FAGX_ConstraintController::FAGX_ConstraintController(bool bInRotational)
	: bEnable(false)
	, Compliance(ConstraintConstants::DefaultCompliance())
	, Damping(ConstraintConstants::DefaultDamping())
	, ForceRange(ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax())
	, bRotational(bInRotational)
	, NativeBarrier(nullptr)
{
}

FAGX_ConstraintController::~FAGX_ConstraintController()
{
}

FAGX_ConstraintController& FAGX_ConstraintController::operator=(
	const FAGX_ConstraintController& Other)
{
	bEnable = Other.bEnable;
	Compliance = Other.Compliance;
	Damping = Other.Damping;
	ForceRange = Other.ForceRange;
	bRotational = Other.bRotational;
	return *this;
}

namespace FAGX_ConstraintController_helpers
{
	void PrintNoNativeConstraintWarning()
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ConstraintController without a native constraint used.\nThis happens when a "
				 "ConstraintController is stored in a Blueprint variable. Instead, store a "
				 "reference to the constraint and re-fetch the ConstraintController every time "
				 "it's needed."));
	}
}

void FAGX_ConstraintController::SetEnable(bool bInEnable)
{
	if (HasNative())
	{
		NativeBarrier->SetEnable(bInEnable);
	}
	bEnable = bInEnable;
}

bool FAGX_ConstraintController::GetEnable() const
{
	if (HasNative())
	{
		return NativeBarrier->GetEnable();
	}
	else
	{
		return bEnable;
	}
}

void FAGX_ConstraintController::SetCompliance(double InCompliance)
{
	if (HasNative())
	{
		NativeBarrier->SetCompliance(InCompliance);
	}
	Compliance = InCompliance;
}

double FAGX_ConstraintController::GetCompliance() const
{
	if (HasNative())
	{
		return NativeBarrier->GetCompliance();
	}
	else
	{
		return Compliance;
	}
}


void FAGX_ConstraintController::SetDamping(double InDamping)
{
	if (HasNative())
	{
		NativeBarrier->SetDamping(InDamping);
	}
	Damping = InDamping;
}

double FAGX_ConstraintController::GetDamping() const
{
	if (HasNative())
	{
		return NativeBarrier->GetDamping();
	}
	else
	{
		return Damping;
	}
}

void FAGX_ConstraintController::SetForceRange(const FFloatInterval& InForceRange)
{
	if (HasNative())
	{
		NativeBarrier->SetForceRange(InForceRange);
	}
	ForceRange = InForceRange;
}

FFloatInterval FAGX_ConstraintController::GetForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier->GetForceRange();
	}
	else
	{
		return ForceRange;
	}
}

double FAGX_ConstraintController::GetForce()
{
	if (!HasNative())
	{
		FAGX_ConstraintController_helpers::PrintNoNativeConstraintWarning();
		return 0.0f;
	}
	return NativeBarrier->GetForce();
}

bool FAGX_ConstraintController::HasNative() const
{
	return NativeBarrier.IsValid() && NativeBarrier->HasNative();
}

FConstraintControllerBarrier* FAGX_ConstraintController::GetNative()
{
	check(HasNative());
	return NativeBarrier.Get();
}

const FConstraintControllerBarrier* FAGX_ConstraintController::GetNative() const
{
	check(HasNative());
	return NativeBarrier.Get();
}

void FAGX_ConstraintController::UpdateNativeProperties()
{
	if (!HasNative())
	{
		FAGX_ConstraintController_helpers::PrintNoNativeConstraintWarning();
		return;
	}
	NativeBarrier->SetEnable(bEnable);
	NativeBarrier->SetCompliance(Compliance);
	NativeBarrier->SetDamping(Damping);
	NativeBarrier->SetForceRange(ForceRange);
	UpdateNativePropertiesImpl();
}

void FAGX_ConstraintController::CopyFrom(const FConstraintControllerBarrier& Source)
{
	bEnable = Source.GetEnable();
	Compliance = Source.GetCompliance();
	Damping = Source.GetDamping();
	ForceRange = Source.GetForceRange();
}
