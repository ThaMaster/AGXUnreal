#include "Utilities/AGX_PropertyUtilities.h"


bool
FAGX_PropertyUtilities::PropertyEquals(const TSharedPtr<IPropertyHandle> &First, const TSharedPtr<IPropertyHandle> &Second)
{
	return
		First->IsValidHandle() &&
		Second->IsValidHandle() &&
		First->GetProperty() == Second->GetProperty();
}

UObject* FAGX_PropertyUtilities::GetParentObjectOfStruct(const TSharedPtr<IPropertyHandle> &StructPropertyHandle)
{
	TArray<UObject*> OuterObjects;
	StructPropertyHandle->GetOuterObjects(OuterObjects);

	return OuterObjects.Num() > 0 ? OuterObjects[0] : nullptr;
}

UObject* FAGX_PropertyUtilities::GetObjectFromHandle(const TSharedPtr<IPropertyHandle> &PropertyHandle)
{
	if (PropertyHandle &&
		PropertyHandle->IsValidHandle() &&
		PropertyHandle->GetProperty() &&
		PropertyHandle->GetProperty()->IsA(UObjectProperty::StaticClass()))
	{
		UObject* Object = nullptr;
		if (PropertyHandle->GetValue(Object) != FPropertyAccess::Result::Fail)
		{
			return Object;
		}
	}
	return nullptr;
}
