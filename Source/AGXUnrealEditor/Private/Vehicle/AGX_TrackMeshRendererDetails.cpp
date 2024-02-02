// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackMeshRendererDetails.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackMeshRenderer.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_TrackMeshRendererDetails"

TSharedRef<IDetailCustomization> FAGX_TrackMeshRendererDetails::MakeInstance()
{
	return MakeShareable(new FAGX_TrackMeshRendererDetails);
}

void FAGX_TrackMeshRendererDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory(
		"AGX Track Mesh Renderer", FText::GetEmpty(), ECategoryPriority::Transform);

	TArray<TWeakObjectPtr<UObject>> Objects;
	InDetailBuilder.GetObjectsBeingCustomized(Objects);

	// clang-format off
	CategoryBuilder.AddCustomRow(FText()).WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("UpdatePreviewText", "Update Visual"))
			.OnClicked_Lambda([Objects]()
			{
				for (const TWeakObjectPtr<UObject>& Object : Objects)
				{
					UAGX_TrackMeshRenderer* Renderer = Cast<UAGX_TrackMeshRenderer>(Object.Get());
					if (IsValid(Renderer))
					{
						if (UAGX_TrackComponent* Track = Renderer->FindTargetTrack())
						{
							// Mark Track Preview Data for update.
							Track->RaiseTrackPreviewNeedsUpdate();
						}
						// Get updated Track Preview Data and synchronize renderer data.
						Renderer->RebindToTrackPreviewNeedsUpdateEvent(false);
						Renderer->SynchronizeVisuals();
					}
				}
				return FReply::Handled();
			})
		]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
