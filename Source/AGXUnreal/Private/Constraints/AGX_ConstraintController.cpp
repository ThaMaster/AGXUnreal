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
	, Elasticity(ConstraintConstants::DefaultElasticity())
	, SpookDamping(ConstraintConstants::DefaultSpookDamping())
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
	Elasticity = Other.Elasticity;
	SpookDamping = Other.SpookDamping;
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
	SetElasticity(1.0 / InCompliance);
}

double FAGX_ConstraintController::GetCompliance() const
{
	return 1.0 / GetElasticity();
}

void FAGX_ConstraintController::SetElasticity(double InElasticity)
{
	if (HasNative())
	{
		NativeBarrier->SetElasticity(InElasticity);
	}
	Elasticity = InElasticity;
}

double FAGX_ConstraintController::GetElasticity() const
{
	if (HasNative())
	{
		return NativeBarrier->GetElasticity();
	}
	else
	{
		return Elasticity;
	}
}

void FAGX_ConstraintController::SetSpookDamping(double InSpookDamping)
{
	if (HasNative())
	{
		NativeBarrier->SetSpookDamping(InSpookDamping);
	}
	SpookDamping = InSpookDamping;
}

double FAGX_ConstraintController::GetSpookDamping() const
{
	if (HasNative())
	{
		return NativeBarrier->GetSpookDamping();
	}
	else
	{
		return SpookDamping;
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
	NativeBarrier->SetElasticity(Elasticity);
	NativeBarrier->SetSpookDamping(SpookDamping);
	NativeBarrier->SetForceRange(ForceRange);
	UpdateNativePropertiesImpl();
}

void FAGX_ConstraintController::CopyFrom(const FConstraintControllerBarrier& Source)
{
	bEnable = Source.GetEnable();
	Elasticity = Source.GetElasticity();
	SpookDamping = Source.GetSpookDamping();
	ForceRange = Source.GetForceRange();
}
