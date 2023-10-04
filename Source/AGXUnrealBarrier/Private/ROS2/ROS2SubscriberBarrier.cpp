// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/ROS2SubscriberBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"

// Helper macros to minimize amount of code needed in large switch-statement.
// clang-format off
#define AGX_RECEIVE_ROS2_MSGS(SubType, MsgTypeUnreal, MsgTypeROS2)                          \
{                                                                                           \
	MsgTypeROS2 MsgROS2;                                                                      \
	if (auto Sub = dynamic_cast<const SubType*>(Native.get()))                                \
	{                                                                                         \
		if (Sub->Native->receiveMessage(MsgROS2))                                               \
		{                                                                                       \
			*static_cast<MsgTypeUnreal*>(&OutMsg) = Convert(MsgROS2);                             \
			return true;                                                                          \
		}                                                                                       \
	}                                                                                         \
	else                                                                                      \
	{                                                                                         \
		UE_LOG(                                                                                 \
			LogAGX, Error,                                                                        \
			TEXT("Unexpected internal error: unable to downcast to the correct Subscriber type "  \
				 "in FROS2SubscriberBarrier::ReceiveMessage. No message could not be received."));  \
	}                                                                                         \
	return false;                                                                             \
}

#define AGX_ASSIGN_ROS2_NATIVE(SubTypeUnreal, SubTypeROS2)                                   \
{                                                                                            \
	Native = std::make_unique<SubTypeUnreal>(new SubTypeROS2(Convert(Topic), Convert(Qos)));   \
	return;                                                                                    \
}
// clang-format on

FROS2SubscriberBarrier::FROS2SubscriberBarrier()
{
}

FROS2SubscriberBarrier::~FROS2SubscriberBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration, of FROS2Subscriber.
}

FROS2SubscriberBarrier::FROS2SubscriberBarrier(FROS2SubscriberBarrier&& Other)
{
	*this = std::move(Other);
}

FROS2SubscriberBarrier& FROS2SubscriberBarrier::operator=(FROS2SubscriberBarrier&& Other)
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	MessageType = Other.MessageType;
	return *this;
}

bool FROS2SubscriberBarrier::HasNative() const
{
	return Native != nullptr;
}

void FROS2SubscriberBarrier::AllocateNative(
	EAGX_ROS2MessageType InMessageType, const FString& Topic, const FAGX_ROS2Qos& Qos)
{
	using namespace agxIO::ROS2::stdMsgs;

	MessageType = InMessageType;
	const auto QOSROS2 = Convert(Qos);
	const auto TopicROS2 = Convert(Topic);
	switch (InMessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberFloat32, SubscriberFloat32)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt32, SubscriberInt32)
			return;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported or invalid type passed to FROS2SubscriberBarrier::AllocateNative, topic: "
			 "'%s'. Native will not be allocated."),
		*Topic);
}

FROS2Subscriber* FROS2SubscriberBarrier::GetNative()
{
	return Native.get();
}

const FROS2Subscriber* FROS2SubscriberBarrier::GetNative() const
{
	return Native.get();
}

EAGX_ROS2MessageType FROS2SubscriberBarrier::GetMessageType() const
{
	return MessageType;
}

void FROS2SubscriberBarrier::ReleaseNative()
{
	Native = nullptr;
	MessageType = EAGX_ROS2MessageType::Invalid;
}

bool FROS2SubscriberBarrier::ReceiveMessage(FAGX_ROS2Message& OutMsg) const
{
	using namespace agxIO::ROS2::stdMsgs;
	check(HasNative());

	switch (MessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberFloat32, FAGX_StdMsgsFloat32, Float32)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberInt32, FAGX_StdMsgsInt32, Int32)
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FROS2SubscriberBarrier::ReceivedMessage called on SubscriberBarrier with an invalid "
			 "MessageType. The message will not be received."));
	return false;
}
