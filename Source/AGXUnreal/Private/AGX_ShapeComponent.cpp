#include "AGX_ShapeComponent.h"

#include "AGX_LogCategory.h"

// Sets default values for this component's properties
UAGX_ShapeComponent::UAGX_ShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	UE_LOG(LogAGX, Log, TEXT("ShapeComponent instance crated."));
}

bool UAGX_ShapeComponent::HasNative() const
{
	return GetNative() != nullptr;
}

void UAGX_ShapeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/// \todo Do we need TickComponent on UAGX_ShapeComponent?

	UE_LOG(LogAGX, Log, TEXT("TickComponent for ShapeComponent."));
}

// Called when the game starts
void UAGX_ShapeComponent::BeginPlay()
{
	UE_LOG(LogAGX, Log, TEXT("BeginPlay for ShapeComponent"));
	Super::BeginPlay();
	GetOrCreateNative();
}

void UAGX_ShapeComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	UE_LOG(LogAGX, Log, TEXT("EndPlay for ShapeComponent"));
	Super::EndPlay(Reason);
	ReleaseNative();
}
