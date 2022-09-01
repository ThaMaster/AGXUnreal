// Copyright 2022, Algoryx Simulation AB.

#pragma once
#include <type_traits>


// clang-format off

#define AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName, PreSetCast) \
{ \
	if (IsInstanceAGX()) \
	{ \
		UpropertyName = PreSetCast(InVar); \
		if (HasNativeFunc()) \
		{ \
			NativeName->SetFunc(PreSetCast(InVar)); \
		} \
	} \
	else \
	{ \
		if (Instance != nullptr) \
		{ \
			Instance->SetFunc(InVar); \
			return; \
		} \
		UpropertyName = PreSetCast(InVar); \
	} \
}

#define AGX_ASSET_SETTER_IMPL(UpropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNative, NativeBarrier, /*no cast*/)

#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName, /*no cast*/)

#define AGX_ASSET_SETTER_F2D_IMPL(UpropertyName, InVar, SetFunc) \
	static_assert(std::is_same<decltype(UpropertyName), FAGX_Real>::value || \
		std::is_same<decltype(UpropertyName), double>::value, "UpropertyName must be of FAGX_Real or double type."); \
	static_assert(std::is_same<decltype(InVar), float>::value, "InVar must be of float type."); \
	AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNative, NativeBarrier, static_cast<double>)

#define AGX_ASSET_SETTER_DUAL_NATIVE_F2D_IMPL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	static_assert(std::is_same<decltype(UpropertyName), FAGX_Real>::value || \
		std::is_same<decltype(UpropertyName), double>::value, "UpropertyName must be of FAGX_Real or double type."); \
	static_assert(std::is_same<decltype(InVar), float>::value, "InVar must be of float type."); \
	AGX_ASSET_SETTER_IMPL_INTERNAL(UpropertyName, InVar, SetFunc, HasNativeFunc, NativeName, static_cast<double>)




#define AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNativeFunc, NativeName, PreReturnCast) \
{ \
	if (Instance != nullptr) \
	{ \
		return Instance->GetFunc(); \
	} \
	if (HasNativeFunc()) \
	{ \
		return PreReturnCast(NativeName->GetFunc()); \
	} \
	return PreReturnCast(UpropertyName); \
}

#define AGX_ASSET_GETTER_IMPL(UpropertyName, GetFunc) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNative, NativeBarrier, /*no cast*/)

#define AGX_ASSET_GETTER_DUAL_NATIVE_IMPL(UpropertyName, GetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNativeFunc, NativeName, /*no cast*/)

#define AGX_ASSET_GETTER_D2F_IMPL(UpropertyName, GetFunc) \
	static_assert(std::is_same<decltype(UpropertyName), FAGX_Real>::value || \
		std::is_same<decltype(UpropertyName), double>::value, "UpropertyName must be of FAGX_Real or double type."); \
	AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNative, NativeBarrier, static_cast<float>)

#define AGX_ASSET_GETTER_DUAL_NATIVE_F2D_IMPL(UpropertyName, GetFunc, HasNativeFunc, NativeName) \
	static_assert(std::is_same<decltype(UpropertyName), FAGX_Real>::value || \
		std::is_same<decltype(UpropertyName), double>::value, "UpropertyName must be of FAGX_Real or double type."); \
	AGX_ASSET_GETTER_IMPL_INTERNAL(UpropertyName, GetFunc, HasNativeFunc, NativeName, static_cast<float>)



#define AGX_ASSET_DISPATCHER_LAMBDA_BODY_INTERNAL(UpropertyName, SetFunc, PrePassCast) \
{ \
	if (This->IsInstanceAGX()) \
	{ \
		This->Asset->UpropertyName = This->UpropertyName; \
	} \
	This->SetFunc(PrePassCast(This->UpropertyName)); \
}

#define AGX_ASSET_DISPATCHER_LAMBDA_BODY(UpropertyName, SetFunc) \
	AGX_ASSET_DISPATCHER_LAMBDA_BODY_INTERNAL(UpropertyName, SetFunc, /*no cast*/)

#define AGX_ASSET_DISPATCHER_D2F_LAMBDA_BODY(UpropertyName, SetFunc) \
	static_assert(std::is_same<decltype(UpropertyName), FAGX_Real>::value || \
		std::is_same<decltype(UpropertyName), double>::value, "UpropertyName must be of FAGX_Real or double type."); \
	AGX_ASSET_DISPATCHER_LAMBDA_BODY_INTERNAL(UpropertyName, SetFunc, static_cast<float>)

// clang-format on
