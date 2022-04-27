// Copyright 2022, Algoryx Simulation AB.

#include "AGX_RealInterval.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/DoubleInterval.h"

#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Math/Interval.h"

bool FAGX_RealInterval::SerializeFromMismatchedTag(
	struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot)
{
	if (Tag.Type != NAME_StructProperty)
	{
		return false;
	}

	UScriptStruct* FloatStruct = TBaseStructure<FFloatInterval>::Get();
	if (Tag.StructName == FloatStruct->GetFName())
	{
		// The archive has a Float Interval, conversion needed.
		FFloatInterval Restored;
		FloatStruct->SerializeItem(Slot, &Restored, nullptr);
		Min = static_cast<double>(Restored.Min);
		Max = static_cast<double>(Restored.Max);
		return true;
	}

	UScriptStruct* DoubleStruct = FAGX_DoubleInterval::StaticStruct();
	if (Tag.StructName == DoubleStruct->GetFName())
	{
		/// @todo Make sure this code works. What we can test on? Where is FAGX_DoubleInterval used
		/// as a Property?
		FAGX_DoubleInterval Restored;
		DoubleStruct->SerializeItem(Slot, &Restored, nullptr);
		Min = Restored.Min;
		Max = Restored.Max;
		return true;
	}

	return false;
}
