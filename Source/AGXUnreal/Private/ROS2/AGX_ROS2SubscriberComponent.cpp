// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2SubscriberComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"

UAGX_ROS2SubscriberComponent::UAGX_ROS2SubscriberComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgFloat32(
	FAGX_StdMsgsFloat32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt32(
	FAGX_StdMsgsInt32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

FROS2SubscriberBarrier* UAGX_ROS2SubscriberComponent::GetOrCreateBarrier(
	EAGX_ROS2MessageType Type, const FString& Topic)
{
	FROS2SubscriberBarrier* Barrier = NativeBarriers.Find(Topic);
	if (Barrier == nullptr)
	{
		if (Topic.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateBarrier was called on ROS2 Subscriber Component '%s' whith an "
					 "empty Topic String. Ensure a Topic has been set."),
				*GetName());
			return nullptr;
		}

		bool bIsPlaying = GetWorld() != nullptr && GetWorld()->IsGameWorld();
		if (!bIsPlaying)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateBarrier was called on ROS2 Subscriber Component '%s' when not in "
					 "Play. Only call this function during Play."),
				*GetName());
			return nullptr;
		}

		Barrier = &NativeBarriers.Add(Topic, FROS2SubscriberBarrier());
		Barrier->AllocateNative(Type, Topic, Qos);
	}
	else if (Barrier->GetMessageType() != Type)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Existing Native ROS2 Subscriber with different message type found in "
				 "UAGX_ROS2Subscriber::GetOrCreateBarrier for Topic: '%s'. Ensure only single "
				 "message types are used for a specific Topic."));
		return nullptr;
	}

	AGX_CHECK(Barrier->HasNative());
	return Barrier;
}

bool UAGX_ROS2SubscriberComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty->GetFName().IsEqual(GET_MEMBER_NAME_CHECKED(UAGX_ROS2SubscriberComponent, Qos)))
	{
		UWorld* World = GetWorld();
		return World == nullptr || !World->IsGameWorld();
	}

	return SuperCanEditChange;
}
