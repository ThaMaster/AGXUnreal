// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarSensorComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_LidarSensorComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_LidarSensorComponentCustomization);
}

void FAGX_LidarSensorComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_LidarSensorComponent* Lidar =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_LidarSensorComponent>(
			InDetailBuilder);
	if (Lidar == nullptr)
	{
		return;
	}

	// Hide the default Model widget, we will create a custom one for it.
	InDetailBuilder.HideProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, Model)));

	IDetailCategoryBuilder& CategoryBuilder =
		InDetailBuilder.EditCategory("AGX Lidar", FText::GetEmpty(), ECategoryPriority::Important);

	//clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()[SNew(STextBlock)
						   .Text(LOCTEXT("LidarModel", "Model"))
						   .Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
			[SNew(SComboBox<TSharedPtr<EAGX_LidarModel>>)
				 .ContentPadding(2)
				 .OptionsSource(GetAvailableModels())
				 .OnGenerateWidget_Lambda(
					 [=](TSharedPtr<EAGX_LidarModel> Item)
					 {
						 // content for each item in combo box
						 return SNew(STextBlock)
							 .Text(FText::FromString(
								 FAGX_LidarSensorComponentCustomization::FromEnum(*Item)))
							 .ToolTipText(FText::GetEmpty());
					 })
				 .OnSelectionChanged(
					 this, &FAGX_LidarSensorComponentCustomization::OnModelComboBoxChanged)
				 .Content() // header content showing selected item, even while combo box is closed.
					 [SNew(STextBlock)
						  .Text_Lambda(
							  [this]() {
								  return FText::FromString(
									  FAGX_LidarSensorComponentCustomization::FromEnum(
										  SelectedModel));
							  })]];
	//clang-format on

	InDetailBuilder.HideCategory(FName("Sockets"));
}

void FAGX_LidarSensorComponentCustomization::OnModelComboBoxChanged(
	TSharedPtr<EAGX_LidarModel> NewModel, ESelectInfo::Type InSeletionInfo)
{
	const FText SetModelYesNo = LOCTEXT(
		"LidarSetModelYesNo",
		"This action will overwrite the Lidar properties of this Component and any instance of "
		"this Component. Continue?");

	UAGX_LidarSensorComponent* Lidar =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_LidarSensorComponent>(
			*DetailBuilder);
	if (Lidar == nullptr || NewModel == nullptr)
	{
		return;
	}

	if (!FAGX_NotificationUtilities::YesNoQuestion(SetModelYesNo))
		return;

	Lidar->SetModel(*NewModel);
	SelectedModel = *NewModel;

	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Lidar))
	{
		Instance->CopyFrom(*Lidar);
	}
}

const TArray<TSharedPtr<EAGX_LidarModel>>*
FAGX_LidarSensorComponentCustomization::GetAvailableModels()
{
	if (AvailableModels.Num() == 0)
	{
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::GenericHorizontalSweep));
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::Custom));
	}

	return &AvailableModels;
}

FString FAGX_LidarSensorComponentCustomization::FromEnum(EAGX_LidarModel Model)
{
	switch (Model)
	{
		case EAGX_LidarModel::GenericHorizontalSweep:
			return "GenericHorizontalSweep";
		case EAGX_LidarModel::Custom:
			return "Custom";
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unknown EAGX_LidarModel enum literal passed to "
			 "FAGX_LidarSensorComponentCustomization::FromEnum. Returning 'None'."));
	return "None";
}

#undef LOCTEXT_NAMESPACE
