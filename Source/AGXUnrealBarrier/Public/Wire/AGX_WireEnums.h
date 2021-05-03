#pragma once

#include "AGX_WireEnums.generated.h"

/**
 * Types of wire nodes we support in AGX Dynamics for Unreal.
 *
 * Suitable for array indexing.
 */
UENUM(BlueprintType)
enum class EWireNodeType : uint8
{
	Free,
	Eye,
	BodyFixed,
	NUM_USER_CREATABLE,
	Other // Any node type in AGX Dynamics that we don't allow a user to create directly.
};


/**
 * All wire node types in AGX Dynamics for Unreal.
 *
 * Bitfield, not suitable for array indexing
 */
UENUM()
enum class EWireNodeNativeType : uint8
{
	// These much match the values of agxWire::Node::Type in agxWire/Node.h.
	NOT_DEFINED = 0,
	Eye = (1<<0),
	Missing = (1<<1),
	Connecting = (1<<2),
	Free = (1<<3),
	Contact = (1<<4),
	BodyFixed = (1<<5),
	Stop = (1<<6),
	ShapeContact = (1<<7)
};

