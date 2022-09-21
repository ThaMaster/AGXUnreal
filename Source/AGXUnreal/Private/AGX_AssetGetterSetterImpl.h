// Copyright 2022, Algoryx Simulation AB.

#pragma once
#include <type_traits>


// clang-format off

#define AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
{ \
	if (IsInstance()) \
	{ \
		UpropertyName = InVar; \
		if (HasNativeFunc()) \
		{ \
			NativeName.SetFunc(InVar); \
		} \
	} \
	else \
	{ \
		if (Instance != nullptr) \
		{ \
			Instance->SetFunc(InVar); \
			return; \
		} \
		UpropertyName = InVar; \
	} \
}

#define AGX_ASSET_SETTER_IMPL(UpropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNative, NativeBarrier)

#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName)

#define AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNativeFunc, NativeName) \
{ \
	if (Instance != nullptr) \
	{ \
		return Instance->GetFunc(); \
	} \
	if (HasNativeFunc()) \
	{ \
		return NativeName.GetFunc(); \
	} \
	return UpropertyName; \
}

#define AGX_ASSET_GETTER_IMPL(UpropertyName, GetFunc) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNative, NativeBarrier)

#define AGX_ASSET_GETTER_DUAL_NATIVE_IMPL(UpropertyName, GetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNativeFunc, NativeName)



#define AGX_ASSET_DISPATCHER_LAMBDA_BODY(UpropertyName, SetFunc) \
{ \
	if (This->IsInstance()) \
	{ \
		This->Asset->UpropertyName = This->UpropertyName; \
	} \
	This->SetFunc(This->UpropertyName); \
}

// clang-format on
