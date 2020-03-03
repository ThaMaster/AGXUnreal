#include "Utilities/AGX_PropertyUtilities.h"

// Unreal Engine includes.
#include "UObject/MetaData.h"
#include "UObject/Package.h"

bool FAGX_PropertyUtilities::PropertyEquals(
	const TSharedPtr<IPropertyHandle>& First, const TSharedPtr<IPropertyHandle>& Second)
{
	return First->IsValidHandle() && Second->IsValidHandle() &&
		   First->GetProperty() == Second->GetProperty();
}

UObject* FAGX_PropertyUtilities::GetParentObjectOfStruct(
	const TSharedPtr<IPropertyHandle>& StructPropertyHandle)
{
	TArray<UObject*> OuterObjects;
	StructPropertyHandle->GetOuterObjects(OuterObjects);

	return OuterObjects.Num() > 0 ? OuterObjects[0] : nullptr;
}

UObject* FAGX_PropertyUtilities::GetObjectFromHandle(
	const TSharedPtr<IPropertyHandle>& PropertyHandle)
{
	if (PropertyHandle && PropertyHandle->IsValidHandle() && PropertyHandle->GetProperty() &&
		PropertyHandle->GetProperty()->IsA(UObjectProperty::StaticClass()))
	{
		UObject* Object = nullptr;
		if (PropertyHandle->GetValue(Object) != FPropertyAccess::Result::Fail)
		{
			/// \todo Make sure we get the correct result even when multiple objects are selected. I
			/// worry that we should test for equality with FPRopertyAccess::Success instead of
			/// inequality with FPropertyAccess::Fail because of the possibility of MultipleValues.
			return Object;
		}
	}
	return nullptr;
}

FString FAGX_PropertyUtilities::GetActualDisplayName(const UField* Field, bool bRemoveAgxPrefix)
{
	FString Name;

	if (Field)
	{
		if (Field->HasMetaData(TEXT("DisplayName")))
		{
			Name = Field->GetMetaData(TEXT("DisplayName"));
		}
		else
		{
			Name = FName::NameToDisplayString(
				Field->GetFName().ToString(), Field->IsA(UBoolProperty::StaticClass()));
		}

		if (bRemoveAgxPrefix)
		{
			Name.RemoveFromStart("AGX", ESearchCase::CaseSensitive);
			Name.TrimStartAndEndInline();
		}
	}

	return Name;
}
