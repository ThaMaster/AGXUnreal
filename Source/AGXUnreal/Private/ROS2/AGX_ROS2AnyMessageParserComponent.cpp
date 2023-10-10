#include "ROS2/AGX_ROS2AnyMessageParserComponent.h"

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"

UAGX_ROS2AnyMessageParserComponent::UAGX_ROS2AnyMessageParserComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ROS2AnyMessageParserComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FROS2AnyMessageParserBarrier* UAGX_ROS2AnyMessageParserComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FROS2AnyMessageParserBarrier* UAGX_ROS2AnyMessageParserComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_ROS2AnyMessageParserComponent::BeginPlay()
{
	Super::BeginPlay();
	check(!HasNative());
	NativeBarrier.AllocateNative();
}

void UAGX_ROS2AnyMessageParserComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
		NativeBarrier.ReleaseNative();
}

namespace AGX_ROS2AnyMessageParserComponent_helpers
{
	void PrintMissingNativeWarning(
		const FString& FunctionName, const UAGX_ROS2AnyMessageParserComponent& Component)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' was called on Any Message Parser '%s' that does not have a Native "
				 "object. Only call this function during Play."),
			*FunctionName, *Component.GetName());
	}
}

UAGX_ROS2AnyMessageParserComponent* UAGX_ROS2AnyMessageParserComponent::BeginParse(
	const FAGX_AgxMsgsAny& Message)
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("BeginParse", *this);
		return this;
	}

	NativeBarrier.BeginParse(Message);
	return this;
}

int32 UAGX_ROS2AnyMessageParserComponent::ReadInt8()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt8", *this);
		return 0.0;
	}

	return NativeBarrier.ReadInt8();
}
