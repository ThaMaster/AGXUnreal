// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2Publisher.h"

UAGX_ROS2Publisher::UAGX_ROS2Publisher()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_ROS2Publisher::SendStdMsgsFloat32(const FAGX_StdMsgsFloat32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32, Topic))
		Barrier->SendMessage(Msg);
}

void UAGX_ROS2Publisher::SendStdMsgsInt32(const FAGX_StdMsgsInt32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32, Topic))
		Barrier->SendMessage(Msg);
}

FROS2PublisherBarrier* UAGX_ROS2Publisher::GetOrCreateBarrier(
	EAGX_ROS2MessageType Type, const FString& Topic)
{
	bool bIsPlaying = GetWorld() != nullptr && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetOrCreateBarrier was called on ROS2 Publisher Component '%s' when not in Play. "
				 "Only call this function during Play."),
			*GetName());
		return nullptr;
	}

	FROS2PublisherBarrier* Barrier = NativeBarriers.Find(Topic);
	if (Barrier == nullptr)
	{
		Barrier = &NativeBarriers.Add(Topic, FROS2PublisherBarrier());
		Barrier->AllocateNative(Type, Topic);
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
