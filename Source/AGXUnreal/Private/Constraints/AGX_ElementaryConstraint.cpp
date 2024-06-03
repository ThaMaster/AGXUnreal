// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_ElementaryConstraint.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.

FAGX_ElementaryConstraint::FAGX_ElementaryConstraint()
{
}

FAGX_ElementaryConstraint::~FAGX_ElementaryConstraint()
{
}

void FAGX_ElementaryConstraint::SetEnable(bool bInEnable)
{
	if (HasNative())
	{
		NativeBarrier.SetEnable(bInEnable);
	}
	bEnable = bInEnable;
}

bool FAGX_ElementaryConstraint::GetEnable() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnable();
	}
	return bEnable;
}

void FAGX_ElementaryConstraint::SetCompliance(double InCompliance)
{
	if (HasNative())
	{
		NativeBarrier.SetCompliance(InCompliance);
	}
	Compliance = InCompliance;
}

double FAGX_ElementaryConstraint::GetCompliance() const
{
	if (HasNative())
	{
		return NativeBarrier.GetCompliance();
	}
	return Compliance;
}

void FAGX_ElementaryConstraint::SetElasticity(double InElasticity)
{
	if (HasNative())
	{
		// Elasticity is a dependent property on compliance.
		NativeBarrier.SetElasticity(InElasticity);
		Compliance = NativeBarrier.GetCompliance();
	}
	else
	{
		if (InElasticity > DBL_EPSILON)
		{
			Compliance = 1.0 / InElasticity;
		}
	}
}

double FAGX_ElementaryConstraint::GetElasticity() const
{
	if (HasNative())
	{
		return NativeBarrier.GetElasticity();
	}
	if (Compliance > DBL_EPSILON)
	{
		return 1.0 / Compliance;
	}
	else
	{
		return std::numeric_limits<double>::infinity();
	}
}

void FAGX_ElementaryConstraint::SetSpookDamping(double InSpookDamping)
{
	if (HasNative())
	{
		NativeBarrier.SetSpookDamping(InSpookDamping);
	}
	SpookDamping = InSpookDamping;
}

double FAGX_ElementaryConstraint::GetSpookDamping() const
{
	if (HasNative())
	{
		return NativeBarrier.GetSpookDamping();
	}
	return SpookDamping;
}

void FAGX_ElementaryConstraint::SetForceRange(const FAGX_RealInterval& InForceRange)
{
	if (HasNative())
	{
		NativeBarrier.SetForceRange(InForceRange);
	}
	ForceRange = InForceRange;
}

void FAGX_ElementaryConstraint::SetForceRange(const FDoubleInterval& InForceRange)
{
	SetForceRange(FAGX_RealInterval(InForceRange));
}

void FAGX_ElementaryConstraint::SetForceRange(double InMinForce, double InMaxForce)
{
	SetForceRange(FAGX_RealInterval {InMinForce, InMaxForce});
}

void FAGX_ElementaryConstraint::SetForceRangeMin(double InMinForce)
{
	if (HasNative())
	{
		NativeBarrier.SetForceRangeMin(InMinForce);
	}
	ForceRange.Min = InMinForce;
}

void FAGX_ElementaryConstraint::SetForceRangeMax(double InMaxForce)
{
	if (HasNative())
	{
		NativeBarrier.SetForceRangeMax(InMaxForce);
	}
	ForceRange.Max = InMaxForce;
}

FDoubleInterval FAGX_ElementaryConstraint::GetForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier.GetForceRange();
	}
	return ForceRange;
}

double FAGX_ElementaryConstraint::GetForceRangeMin() const
{
	if (HasNative())
	{
		return NativeBarrier.GetForceRangeMin();
	}
	return ForceRange.Min;
}

double FAGX_ElementaryConstraint::GetForceRangeMax() const
{
	if (HasNative())
	{
		return NativeBarrier.GetForceRangeMax();
	}
	return ForceRange.Max;
}

double FAGX_ElementaryConstraint::GetForce()
{
	if (HasNative())
	{
		return NativeBarrier.GetForce();
	}
	return 0.0;
}

bool FAGX_ElementaryConstraint::IsActive() const
{
	if (HasNative())
	{
		return NativeBarrier.IsActive();
	}
	return false;
}

FString FAGX_ElementaryConstraint::GetName() const
{
	if (HasNative())
	{
		return NativeBarrier.GetName();
	}
	return FName(NAME_None).ToString();
}

bool FAGX_ElementaryConstraint::HasNative() const
{
	return NativeBarrier.HasNative();
}

FElementaryConstraintBarrier* FAGX_ElementaryConstraint::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FElementaryConstraintBarrier* FAGX_ElementaryConstraint::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void FAGX_ElementaryConstraint::InitializeBarrier(const FElementaryConstraintBarrier& Barrier)
{
	check(!HasNative());
	NativeBarrier = Barrier;
	check(HasNative());
}

void FAGX_ElementaryConstraint::UpdateNativeProperties()
{
	check(HasNative());
	NativeBarrier.SetEnable(bEnable);
	NativeBarrier.SetCompliance(Compliance);
	NativeBarrier.SetSpookDamping(SpookDamping);
	NativeBarrier.SetForceRange(ForceRange);
}

void FAGX_ElementaryConstraint::CopyFrom(
	const FElementaryConstraintBarrier& Source,
	TArray<FAGX_ElementaryConstraint*>& ArchetypeInstances, bool bForceOverwriteInstances)
{
	const bool bEnableBarrier = Source.GetEnable();
	const FAGX_Real ComplianceBarrier = Source.GetCompliance();
	const FAGX_Real SpookDampingBarrier = Source.GetSpookDamping();
	const FAGX_RealInterval ForceRangeBarrier = Source.GetForceRange();

	for (auto Instance : ArchetypeInstances)
	{
		if (Instance == nullptr)
		{
			continue;
		}

		if (bForceOverwriteInstances)
		{
			Instance->bEnable = bEnableBarrier;
			Instance->Compliance = ComplianceBarrier;
			Instance->SpookDamping = SpookDampingBarrier;
			Instance->ForceRange = ForceRangeBarrier;
		}
		else
		{
			FAGX_ObjectUtilities::SetIfEqual(Instance->bEnable, bEnable, bEnableBarrier);
			FAGX_ObjectUtilities::SetIfEqual(Instance->Compliance, Compliance, ComplianceBarrier);
			FAGX_ObjectUtilities::SetIfEqual(
				Instance->SpookDamping, SpookDamping, SpookDampingBarrier);
			FAGX_ObjectUtilities::SetIfEqual(Instance->ForceRange, ForceRange, ForceRangeBarrier);
		}
	}

	bEnable = bEnableBarrier;
	Compliance = ComplianceBarrier;
	SpookDamping = SpookDampingBarrier;
	ForceRange = ForceRangeBarrier;
}
