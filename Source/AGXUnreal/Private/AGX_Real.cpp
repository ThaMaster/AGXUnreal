// Copyright 2022, Algoryx Simulation AB.

#include <AGX_Real.h>

bool FAGX_Real::SerializeFromMismatchedTag(
	struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot)
{
	if (Tag.Type == NAME_DoubleProperty)
	{
		Slot << Value;
		return true;
	}
	else
	{
		return false;
	}
}
