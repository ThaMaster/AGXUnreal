#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

class IDetailLayoutBuilder;

/**
 *
 */
class FAGX_AgxEdModeFileCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void CustomizeFileImporterCategory(IDetailLayoutBuilder& DetailBuilder);

	void CustomizeFileExporterCategory(IDetailLayoutBuilder& DetailBuilder);

	template <typename Function>
	void AddCustomButton(
		IDetailCategoryBuilder& CategoryBuilder, const FText& ButtonText,
		Function ButtonClickCallbackFunction);
};

template <typename Function>
void FAGX_AgxEdModeFileCustomization::AddCustomButton(
	IDetailCategoryBuilder& CategoryBuilder, const FText& ButtonText,
	Function ButtonClickCallbackFunction)
{
	/** Create custom button */
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth() +
		 SHorizontalBox::Slot().AutoWidth()
			 [SNew(SButton).Text(ButtonText).OnClicked_Lambda(ButtonClickCallbackFunction)]];
}
