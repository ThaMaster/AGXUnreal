// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_Real.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_Real
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	double Value = 0.0;

	FAGX_Real()
	{
	}

	FAGX_Real(double InValue)
		: Value(InValue)
	{
	}

	operator double() const
	{
		return Value;
	}

	operator double&()
	{
		return Value;
	}

	FAGX_Real& operator=(double InValue)
	{
		Value = InValue;
		return *this;
	}

	/// Called by Unreal Engine when de-serializing an FAGX_Real but some other type was found in
	/// the archive. Doubles are read but all other types are rejected.
	bool SerializeFromMismatchedTag(struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot);
};

/**
 * A struct that informn Unreal Engion about how FAGX_Real can be used.
 */
template <>
struct TStructOpsTypeTraits<FAGX_Real> : public TStructOpsTypeTraitsBase2<FAGX_Real>
{
	enum
	{
		/**
		 * Tell Unreal Engine that while restoring an FAGX_Real it is OK to find a double since we
		 * can convert from double to FAGX_Real.
		 */
		WithStructuredSerializeFromMismatchedTag = true

		/// \todo Should more be listed here?
		/// What are the options?
		/// We want to look like a POD type, what should such a type list here?
	};
};
