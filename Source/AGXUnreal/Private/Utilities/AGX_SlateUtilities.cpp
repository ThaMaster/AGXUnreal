#include "Utilities/AGX_SlateUtilities.h"

#include "SBoxPanel.h"
#include "SWidget.h"

bool FAGX_SlateUtilities::RemoveChildWidgetByType(
	const TSharedPtr<SWidget>& Parent, const FString& TypeNameToRemove, bool Recursive)
{
	if (!Parent)
		return false;

	FChildren* Children = Parent->GetChildren();

	for (int32 ChildIndex = 0; ChildIndex < Children->Num(); ++ChildIndex)
	{
		TSharedRef<SWidget> Child = Children->GetChildAt(ChildIndex);

		if (Child->GetTypeAsString() == TypeNameToRemove)
		{
			const FString ParentType = Parent->GetTypeAsString();
			if (ParentType == "SHorizontalBox" || ParentType == "SVerticalBox" || ParentType == "SBoxPanel")
			{
				SBoxPanel* BoxPanel = static_cast<SBoxPanel*>(Parent.Get());

				if (BoxPanel->RemoveSlot(Child) != -1)
					return true;
			}
		}

		if (Recursive && RemoveChildWidgetByType(Child, TypeNameToRemove, Recursive))
			return true;
	}

	return false;
}

void FAGX_SlateUtilities::LogChildWidgets(const TSharedPtr<SWidget>& Parent, bool Recursive, const FString& Prefix)
{
	if (!Parent)
		return;

	FChildren* Children = Parent->GetChildren();

	for (int32 ChildIndex = 0; ChildIndex < Children->Num(); ++ChildIndex)
	{
		TSharedRef<SWidget> Child = Children->GetChildAt(ChildIndex);

		UE_LOG(LogTemp, Log, TEXT("%s%s"), *Prefix, *Child->GetTypeAsString());

		if (Recursive)
			LogChildWidgets(Child, Recursive, Prefix + "  ");
	}
}