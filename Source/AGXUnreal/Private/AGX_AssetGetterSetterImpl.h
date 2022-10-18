// Copyright 2022, Algoryx Simulation AB.

#pragma once
#include <type_traits>

// clang-format off

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

#define AGX_ASSET_SETTER_IMPL_POINTER(PropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNative, NativeBarrier, ->)

#define AGX_ASSET_SETTER_IMPL_VALUE(PropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNative, NativeBarrier, .)

#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_POINTER(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, ->)

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
