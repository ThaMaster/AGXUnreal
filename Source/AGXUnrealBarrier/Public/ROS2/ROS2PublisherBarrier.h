// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Enums.h"

// Standard library includes.
#include <memory>

struct FAGX_ROS2Message;
struct FAGX_ROS2Qos;
struct FROS2Publisher;


class AGXUNREALBARRIER_API FROS2PublisherBarrier
{
public:
	FROS2PublisherBarrier();
	~FROS2PublisherBarrier();
	FROS2PublisherBarrier(FROS2PublisherBarrier&& Other);
	FROS2PublisherBarrier& operator=(FROS2PublisherBarrier&& Other);

	bool HasNative() const;

	void AllocateNative(
		EAGX_ROS2MessageType InMessageType, const FString& Topic, const FAGX_ROS2Qos& Qos);

	FROS2Publisher* GetNative();
	const FROS2Publisher* GetNative() const;

	void ReleaseNative();

	EAGX_ROS2MessageType GetMessageType() const;
	void SendMessage(const FAGX_ROS2Message& Msg) const;

private:
	FROS2PublisherBarrier(const FROS2PublisherBarrier&) = delete;
	void operator=(const FROS2PublisherBarrier&) = delete;

private:
	std::unique_ptr<FROS2Publisher> Native;
	EAGX_ROS2MessageType MessageType {EAGX_ROS2MessageType::Invalid};
};
