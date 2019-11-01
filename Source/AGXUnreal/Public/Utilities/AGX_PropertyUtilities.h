// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PropertyHandle.h"
#include "UnrealType.h"


/**
 * Provides helper functions for working with UProperty and IPropertyHandle.
 */
class AGXUNREAL_API FAGX_PropertyUtilities
{
public:

	static bool	PropertyEquals(const TSharedPtr<IPropertyHandle> &First, const TSharedPtr<IPropertyHandle> &Second);

	static UObject*	GetParentObjectOfStruct(const TSharedPtr<IPropertyHandle> &StructPropertyHandle);

	static UObject* GetObjectFromHandle(const TSharedPtr<IPropertyHandle> &PropertyHandle);

	template<typename TStruct, typename TOwningClass>
	static TStruct* GetStructFromHandle(const TSharedPtr<IPropertyHandle> &PropertyHandle, TOwningClass *OwningClass);
};


template<typename TStruct, typename TOwningClass>
TStruct* FAGX_PropertyUtilities::GetStructFromHandle(const TSharedPtr<IPropertyHandle> &PropertyHandle, TOwningClass *OwningClass)
{
	if (PropertyHandle->IsValidHandle())
		return PropertyHandle->GetProperty()->ContainerPtrToValuePtr<TStruct>(OwningClass);
	else
		return nullptr;
}

