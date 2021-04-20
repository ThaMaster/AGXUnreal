#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// System includes.
#include <cstdint>
#include <limits>

class UActorComponent;

/**
 * Interface implemented by all classes that owns an AGX Dynamics native object.
 *
 * Provides access to the address of the AGX Dynamics object as an unsigned integer. This is used
 * during Blueprint reconstruction (RerunConstructionScripts) when all Blueprint-created Components
 * are serialized, destroyed, and then recreated. We store the address of the AGX Dynamics native
 * object in a Component Instance Data and then give it to the newly created replacement Component
 * instance when the Component Instance Data is applied to it.
 */
class IAGX_NativeOwner
{
public:
	/** \return True if this Native Owner currently owns a native AGX Dynamics object. */
	virtual bool HasNative() const = 0;

	/** \return The address of the currently owned native AGX Dynamics object, or 0. */
	virtual uint64 GetNativeAddress() const = 0;

	/**
	 * Make this Native Owner the owner of the native AGX Dynamics object at the given address.
	 *
	 * The given address must designate a valid AGX Dynamics object of the correct type for the
	 * actual type implementing this interface. The intention is that Unreal Engine's transaction
	 * system is responsible for maintaining the association between the instance GetNativeAddress
	 * is called on and the instance that AssignNative is called on to guarantee type equality.
	 *
	 * @param NativeAddress The address of the native AGX Dynamics object that this Native Owner
	 * should own.
	 */
	virtual void AssignNative(uint64 NativeAddress) = 0;

	//virtual IAGX_NativeOwner* AsNativeOwner(UActorComponent* Component) const = 0;

	virtual ~IAGX_NativeOwner() = default;
};

// Make sure we can hold an address in an Unreal Engine uint64.
static_assert(
	std::numeric_limits<uint64>::max() >= std::numeric_limits<uintptr_t>::max(),
	"The Unreal Engine type uint64 isn't large enough to hold a pointer address. We need an Unreal "
	"Engine type for serialization to work.");
