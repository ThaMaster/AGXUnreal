// Copyright 2023, Algoryx Simulation AB.

#pragma once
#include <type_traits>

/*
 * This file contains a number of helper-macros useful for dealing with our asset/instance type
 * and property updates in general.
 */

// clang-format off

/**
 * @brief Final expansion of the various AGX_ASSET_SETTER macros.
 * @param PropertyName The name of the property to set. May be a StructName.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 * @param HasNativeFunc Function to call in order to determine if the Barrier has a Native.
 * @param NativeName The name of the Barrier member variable.
 * @param BarrierMemberAccess The operator to use to access member functions in the Barrier, either '.' or '->'.
 */
 #define AGX_ASSET_SETTER_IMPL_INTERNAL( \
	PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, BarrierMemberAccess) \
{ \
	if (IsInstance()) \
	{ \
		PropertyName = InVar; \
		if (HasNativeFunc()) \
		{ \
			NativeName BarrierMemberAccess SetFunc(InVar); \
		} \
	} \
	else \
	{ \
		if (Instance != nullptr) \
		{ \
			Instance->SetFunc(InVar); \
		} \
		else \
		{ \
			PropertyName = InVar; \
		} \
	} \
}

/**
 * @brief Set a new Property value on one of our asset/instance types, where the NativeBarrier is a pointer.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_IMPL_POINTER(PropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNative, NativeBarrier, ->)

/**
 * @brief Set a new Property value on one of our asset/instance types, where the NativeBarrier is held by-value.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_IMPL_VALUE(PropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNative, NativeBarrier, .)

/**
 * @brief Set a new Property value on one of our asset/instance types, where there are two NativeBarriers
 * and the one we want is a pointer.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_POINTER(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, ->)

/**
 * @brief Set a new Property value on one of our asset/instance types, where there are two NativeBarriers
 * and the one we want is held by-value.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, .)



#define AGX_ASSET_GETTER_IMPL_INTERNAL( \
	PropertyName, GetFunc, HasNativeFunc, NativeName, BarrierMemberAccess) \
{ \
	if (Instance != nullptr) \
	{ \
		return Instance->GetFunc(); \
	} \
	if (HasNativeFunc()) \
	{ \
		return NativeName BarrierMemberAccess GetFunc(); \
	} \
	return PropertyName; \
}

#define AGX_ASSET_GETTER_IMPL_POINTER(PropertyName, GetFunc) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNative, NativeBarrier, ->)

#define AGX_ASSET_GETTER_IMPL_VALUE(PropertyName, GetFunc) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNative, NativeBarrier, .)

#define AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_POINTER(PropertyName, GetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNativeFunc, NativeName, ->)

#define AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(PropertyName, GetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNativeFunc, NativeName, .)


#define AGX_ASSET_DISPATCHER_LAMBDA_BODY(PropertyName, SetFunc) \
{ \
	if (This->IsInstance()) \
	{ \
		This->Asset->PropertyName = This->PropertyName; \
	} \
	This->SetFunc(This->PropertyName); \
}

// clang-format on
