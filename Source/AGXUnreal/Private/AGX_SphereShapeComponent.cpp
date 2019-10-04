#include "AGX_SphereShapeComponent.h"

#include "AGX_LogCategory.h"

UAGX_SphereShapeComponent::UAGX_SphereShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Radius = 50.0f;
	UE_LOG(LogAGX, Log, TEXT("SphereShape instance created."));
}

FShapeBarrier* UAGX_SphereShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_SphereShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_SphereShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FSphereShapeBarrier* UAGX_SphereShapeComponent::GetNativeSphere()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}


void UAGX_SphereShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for SphereShapeComponent."));
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeTransform(NativeBarrier);
	NativeBarrier.SetRadius(Radius, GetWorld());
}

void UAGX_SphereShapeComponent::ReleaseNative()
{
	UE_LOG(LogAGX, Log, TEXT("Releasing native object for SphereShapeComponent."));
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
