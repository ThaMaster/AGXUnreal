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
	// To follow Unreal Engine convention, we may chose to not initialize Value and instead
	// provide a constructor taking an EForceInit and only do value initialization in that
	// constructor.

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
 * A struct that informs Unreal Engine about how FAGX_Real can be used.
 */
template <>
struct TStructOpsTypeTraits<FAGX_Real> : public TStructOpsTypeTraitsBase2<FAGX_Real>
{
	// clang-format off
	enum
	{
		// These are all the flags that can be set on a TSTructOpsTypeTraits, copied from
		// Engine/Source/Runtime/CoreUObject/Public/UObject/Class.h. They are all false by default,
		// except for WithCopy, which means that any true in this list was set by us and is thus
		// a potential bug.
		//
		// How should this flags be set for this type?

		WithZeroConstructor            = true,                          // struct can be constructed as a valid object by filling its memory footprint with zeroes.
		WithNoInitConstructor          = false,                         // struct has a constructor which takes an EForceInit parameter which will force the constructor to perform initialization, where the default constructor performs 'uninitialization'.
		WithNoDestructor               = true,                          // struct will not have its destructor called when it is destroyed.
		WithCopy                       = !TIsPODType<FAGX_Real>::Value, // struct can be copied via its copy assignment operator.
		WithIdenticalViaEquality       = true,                          // struct can be compared via its operator==.  This should be mutually exclusive with WithIdentical.
		WithIdentical                  = false,                         // struct can be compared via an Identical(const T* Other, uint32 PortFlags) function.  This should be mutually exclusive with WithIdenticalViaEquality.
		WithExportTextItem             = false,                         // struct has an ExportTextItem function used to serialize its state into a string.
		WithImportTextItem             = false,                         // struct has an ImportTextItem function used to deserialize a string into an object of that class.
		WithAddStructReferencedObjects = false,                         // struct has an AddStructReferencedObjects function which allows it to add references to the garbage collector.
		WithSerializer                 = false,                         // struct has a Serialize function for serializing its state to an FArchive.
		WithStructuredSerializer       = false,                         // struct has a Serialize function for serializing its state to an FStructuredArchive.
		WithPostSerialize              = false,                         // struct has a PostSerialize function which is called after it is serialized
		WithNetSerializer              = false,                         // struct has a NetSerialize function for serializing its state to an FArchive used for network replication.
		WithNetDeltaSerializer         = false,                         // struct has a NetDeltaSerialize function for serializing differences in state from a previous NetSerialize operation.
		WithSerializeFromMismatchedTag = false,                         // struct has a SerializeFromMismatchedTag function for converting from other property tags.

		/**
		 * Tell Unreal Engine that while restoring an FAGX_Real it is OK to find a double since we
		 * can convert from double to FAGX_Real. This part doesn't specify the double bit, just that
		 * the struct should at least get the chance to restore itself even when the type in the
		 * archive isn't an FAGX_Real.
		 */
		WithStructuredSerializeFromMismatchedTag = true,               // struct has an FStructuredArchive-based SerializeFromMismatchedTag function for converting from other property tags.
		WithPostScriptConstruct        = false,                        // struct has a PostScriptConstruct function which is called after it is constructed in blueprints
		WithNetSharedSerialization     = false,                        // struct has a NetSerialize function that does not require the package map to serialize its state.
	};
	// clang-format on
};
