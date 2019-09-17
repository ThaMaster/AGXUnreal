#if 0
#include "AGX_BoxShapeComponent.h"

#include "AGX_LogCategory.h"

UAGX_BoxShapeComponent::UAGX_BoxShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogAGX, Log, TEXT("BoxShape instance created."));
}

agx::agxCollide_Box* UAGX_BoxShapeComponent::GetOrCreateBox()
{
	if (Native == nullptr)
	{
		CreateNativeBox();
	}
	return Native;
}

agx::agxCollide_Box* UAGX_BoxShapeComponent::GetBox()
{
	return Native;
}

void UAGX_BoxShapeComponent::CreateNativeShapes(TArray<agx::agxCollide_ShapeRef>& OutNativeShapes)
{
	if (Native == nullptr)
	{
		CreateNativeBox();
	}
	check(Native != nullptr);
	OutNativeShapes.Add(Native);
}

void UAGX_BoxShapeComponent::CreateNativeBox()
{
	Native = agx::allocate(TEXT("agxCollide::Box"));
}
#endif
