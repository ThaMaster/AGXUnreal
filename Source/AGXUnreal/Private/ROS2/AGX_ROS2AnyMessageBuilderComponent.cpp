#include "ROS2/AGX_ROS2AnyMessageBuilderComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "ROS2/AGX_ROS2Messages.h"

UAGX_ROS2AnyMessageBuilderComponent::UAGX_ROS2AnyMessageBuilderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ROS2AnyMessageBuilderComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FROS2AnyMessageBuilderBarrier* UAGX_ROS2AnyMessageBuilderComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FROS2AnyMessageBuilderBarrier* UAGX_ROS2AnyMessageBuilderComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_ROS2AnyMessageBuilderComponent::BeginPlay()
{
	Super::BeginPlay();
	check(!HasNative());
	NativeBarrier.AllocateNative();
}

void UAGX_ROS2AnyMessageBuilderComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
		NativeBarrier.ReleaseNative();
}

namespace AGX_ROS2AnyMessageBuilderComponent_helpers
{
	void PrintMissingNativeWarning(
		const FString& FunctionName, const UAGX_ROS2AnyMessageBuilderComponent& Component)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' was called on Any Message Builder '%s' that does not have a Native "
				 "object. Only call this function during Play."), *FunctionName, *Component.GetName());
	}
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::BeginMessage()
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("BeginMessage", *this);
		return this;
	}

	NativeBarrier.BeginMessage();
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt8(int32 data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt8", *this);
		return this;
	}

	NativeBarrier.WriteInt8(static_cast<int8_t>(data));
	return this;
}

FAGX_AgxMsgsAny UAGX_ROS2AnyMessageBuilderComponent::GetMessage() const
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("GetMessage", *this);
		return FAGX_AgxMsgsAny();
	}

	return NativeBarrier.GetMessage();
}
