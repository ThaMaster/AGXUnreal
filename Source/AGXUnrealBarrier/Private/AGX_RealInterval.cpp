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

	static const FName FloatIntervalName("FloatInterval");
	// static const FName FloatIntervalStructName = FFloatInterval::StaticStruct()->GetFName();
	// UE_LOG(LogAGX, Warning, TEX("Float: Hard-coded name: %s. Struct name: %s"),
	// *FloatIntervalName.ToString(), *FloatIntervalStructName.ToString());

	static const FName DoubleIntervalName("AGX_DoubleInterval");
	static const FName DoubleIntervalStructName = FAGX_DoubleInterval::StaticStruct()->GetFName();
	UE_LOG(
		LogAGX, Warning, TEXT("Double: Hard-coded name: %s. Struct name: %s"),
		*DoubleIntervalName.ToString(), *DoubleIntervalStructName.ToString());

	if (Tag.StructName == FloatIntervalName)
	{
		// The archive has a Float Interval, conversion needed.
		FFloatInterval Restored;

#if 1
		Slot << Restored;
#endif

// This code tries to use the reflection system to read the Float Interval. This is what most
// implementations of SerializeFromMismatchedTag in Unreal Engine does. This fails because
// FFloatInterval isn't marked with USTRUCT, so there is no reflection data.
#if 0
		FFloatInterval::StaticStruct()->SerializeItem(Slot, &Restored, nullptr);
#endif

// This code manually enters a record in the archive and reads the two floating point values. This
// fails because there is more stuff in the archive that should be read, I assume some kind of
// meta-data. So a check in the caller, UStruct::SerializeVersionedTaggedProperties Class.cpp:1499,
// will fail.
#if 0
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		Record << SA_VALUE(TEXT("Min"), Restored.Min);
		Record << SA_VALUE(TEXT("Max"), Restored.Max);
#endif
		Min = static_cast<double>(Restored.Min);
		Max = static_cast<double>(Restored.Max);
		return true;
	}

	if (Tag.StructName == DoubleIntervalName)
	{
		/// @todo Make sure this code works. What we can test on? Where is FAGX_DoubleInterval used
		/// as a Property?
		FAGX_DoubleInterval Restored;
		FAGX_DoubleInterval::StaticStruct()->SerializeItem(Slot, &Restored, nullptr);
		Min = Restored.Min;
		Max = Restored.Max;
		return true;
	}

	return false;
}
