#include "AGX_BoxShapeComponent.h"

#include "AGX_LogCategory.h"


UAGX_BoxShapeComponent::UAGX_BoxShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogAGX, Log, TEXT("BoxShape instance created."));
}

agx::agxCollide_Box* UAGX_BoxShapeComponent::getOrCreateNative()
{
	if (!HasNative())
	{
		InitializeNative();
	}

	return Native;
}

agx::agxCollide_Box* UAGX_BoxShapeComponent::getNative()
{
	return Native;
}

bool UAGX_BoxShapeComponent::HasNative() const
{
	return Native != nullptr;
}

void UAGX_BoxShapeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}

	UE_LOG(LogAGX, Log, TEXT("BoxShape ready to simulate."));
}

void UAGX_BoxShapeComponent::InitializeNative()
{
	NativeGeometry = agx::allocate(TEXT("agxCollide::Geometry"));
	Native = agx::allocate(TEXT("agxCollide::Box"));

	agx::call(TEXT("NativeGeometry->add(Native);"));
}
