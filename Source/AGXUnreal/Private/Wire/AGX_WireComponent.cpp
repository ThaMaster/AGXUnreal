#include "Wire/AGX_WireComponent.h"

#include "Components/BillboardComponent.h"

#define LOCTEXT_NAMESPACE "UAGX_WireComponent"

UAGX_WireComponent::UAGX_WireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bVisualizeComponent = true;
}

void UAGX_WireComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		SpriteComponent->SetSprite(
			LoadObject<UTexture2D>(nullptr, TEXT("/AGXUnreal/Editor/Icons/T_AGX_Wire.T_AGX_Wire")));
	}
#endif
}

void UAGX_WireComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
