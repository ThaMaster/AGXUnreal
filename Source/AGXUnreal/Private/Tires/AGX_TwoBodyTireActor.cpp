#include "AGX_TwoBodyTireActor.h"

// AGXUnreal includes.
#include "AGX_TwoBodyTireComponent.h"

// Unreal Engine includes.
#include "Engine/EngineTypes.h"

AAGX_TwoBodyTireActor::AAGX_TwoBodyTireActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TwoBodyTireComponent =
		CreateDefaultSubobject<UAGX_TwoBodyTireComponent>(TEXT("TwoBodyTireComponent"));
}
