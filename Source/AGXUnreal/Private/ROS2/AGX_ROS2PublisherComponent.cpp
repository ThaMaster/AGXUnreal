// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2PublisherComponent.h"

UAGX_ROS2PublisherComponent::UAGX_ROS2PublisherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsFloat32(
	const FAGX_StdMsgsFloat32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32, Topic))
		return Barrier->SendMessage(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt32(
	const FAGX_StdMsgsInt32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32, Topic))
		return Barrier->SendMessage(Msg);
	return false;
}

FROS2PublisherBarrier* UAGX_ROS2PublisherComponent::GetOrCreateBarrier(
	EAGX_ROS2MessageType Type, const FString& Topic)
{
	FROS2PublisherBarrier* Barrier = NativeBarriers.Find(Topic);
	if (Barrier == nullptr)
	{
		if (Topic.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateBarrier was called on ROS2 Publisher Component '%s' whith an "
					 "empty Topic String. Ensure a Topic has been set."),
				*GetName());
			return nullptr;
		}

		bool bIsPlaying = GetWorld() != nullptr && GetWorld()->IsGameWorld();
		if (!bIsPlaying)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateBarrier was called on ROS2 Publisher Component '%s' when not in "
					 "Play. Only call this function during Play."),
				*GetName());
			return nullptr;
		}

		Barrier = &NativeBarriers.Add(Topic, FROS2PublisherBarrier());
		Barrier->AllocateNative(Type, Topic, Qos);
	}
	else if (Barrier->GetMessageType() != Type)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Existing Native ROS2 Publisher with different message type found in "
				 "UAGX_ROS2Publisher::GetOrCreateBarrier for Topic: '%s'. Ensure only single "
				 "message types are used for a specific Topic."));
		return nullptr;
	}

	AGX_CHECK(Barrier->HasNative());
	return Barrier;
}

bool UAGX_ROS2PublisherComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty->GetFName().IsEqual(GET_MEMBER_NAME_CHECKED(UAGX_ROS2PublisherComponent, Qos)))
	{
		UWorld* World = GetWorld();
		return World == nullptr || !World->IsGameWorld();
	}

	return SuperCanEditChange;
}
