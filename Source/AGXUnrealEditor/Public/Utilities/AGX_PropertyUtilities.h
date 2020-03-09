#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"

class USceneComponent;

/**
 * Provides helper functions for working with UProperty and IPropertyHandle.
 */
class AGXUNREALEDITOR_API FAGX_PropertyUtilities
{
public:
	static bool PropertyEquals(
		const TSharedPtr<IPropertyHandle>& First, const TSharedPtr<IPropertyHandle>& Second);

	static UObject* GetParentObjectOfStruct(
		const TSharedPtr<IPropertyHandle>& StructPropertyHandle);

	static UObject* GetObjectFromHandle(const TSharedPtr<IPropertyHandle>& PropertyHandle);

	template <typename TStruct, typename TOwner>
	static TStruct* GetStructFromHandle(
		const TSharedPtr<IPropertyHandle>& PropertyHandle, TOwner* Owner);

	/**
	 * Returns the Display Name metadata if such exists, or else the name converted to
	 * display format (e.g. adding spacing between words, etc). Does not do localization yet!
	 *
	 * Note that this functions takes UField as input. The reason for that is because most
	 * metadata classes (UProperty, UClass, UStruct) inherit from UField.
	 */
	static FString GetActualDisplayName(const UField* Field, bool bRemoveAgxPrefix);
};

template <typename TStruct, typename TOwningClass>
TStruct* FAGX_PropertyUtilities::GetStructFromHandle(
	const TSharedPtr<IPropertyHandle>& PropertyHandle, TOwningClass* OwningClass)
{
	if (!PropertyHandle->IsValidHandle())
	{
		return nullptr;
	}

	return PropertyHandle->GetProperty()->ContainerPtrToValuePtr<TStruct>(OwningClass);
}
