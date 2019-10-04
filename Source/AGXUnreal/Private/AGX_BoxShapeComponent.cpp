#include "AGX_BoxShapeComponent.h"

#include "AGX_LogCategory.h"

UAGX_BoxShapeComponent::UAGX_BoxShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	HalfExtent = FVector(1.0f, 1.0f, 1.0f);
	UE_LOG(LogAGX, Log, TEXT("BoxShape instance created."));
}

FShapeBarrier* UAGX_BoxShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_BoxShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_BoxShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FBoxShapeBarrier* UAGX_BoxShapeComponent::GetNativeBox()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_BoxShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for BoxShapeComponent."));
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeTransform(NativeBarrier);
	NativeBarrier.SetHalfExtents(HalfExtent, GetWorld());
}

void UAGX_BoxShapeComponent::ReleaseNative()
{
	UE_LOG(LogAGX, Log, TEXT("Releasing native object for BoxShapeComponent."));
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
