// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/ROS2PublisherBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"

// Helper macro to minimize amount of code needed in large switch-statement.
// clang-format off
#define AGX_SEND_ROS2_MSGS(PubType, MsgType)                                            \
{                                                                                       \
	if (auto Pub = dynamic_cast<const PubType*>(Native.get()))                            \
	{                                                                                     \
		Pub->Native->sendMessage(Convert(*static_cast<const MsgType*>(&Msg)));              \
	}                                                                                     \
	else                                                                                  \
	{                                                                                     \
		UE_LOG(                                                                             \
			LogAGX, Error,                                                                    \
			TEXT(                                                                             \
				"Unexpected internal error: unable to downcast to the correct Publisher type "  \
				"in FROS2PublisherBarrier::SendMessage. The message will not be sent."));       \
	}                                                                                     \
	return;                                                                               \
}

#define AGX_ASSIGN_ROS2_NATIVE(PubTypeUnreal, PubTypeROS2)                                   \
{                                                                                            \
	Native = std::make_unique<PubTypeUnreal>(new PubTypeROS2(Convert(Topic), Convert(Qos)));   \
	return;                                                                                    \
}
// clang-format on

FROS2PublisherBarrier::FROS2PublisherBarrier()
{
}

FROS2PublisherBarrier::~FROS2PublisherBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration, of FROS2Publisher.
}

FROS2PublisherBarrier::FROS2PublisherBarrier(FROS2PublisherBarrier&& Other)
{
	*this = std::move(Other);
}

FROS2PublisherBarrier& FROS2PublisherBarrier::operator=(FROS2PublisherBarrier&& Other)
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	MessageType = Other.MessageType;
	return *this;
}

bool FROS2PublisherBarrier::HasNative() const
{
	return Native != nullptr;
}

void FROS2PublisherBarrier::AllocateNative(
	EAGX_ROS2MessageType InMessageType, const FString& Topic, const FAGX_ROS2Qos& Qos)
{
	using namespace StdMsgs;
	using namespace agxIO::ROS2::stdMsgs;

	MessageType = InMessageType;
	const auto QOSROS2 = Convert(Qos);
	const auto TopicROS2 = Convert(Topic);
	switch (InMessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherFloat32, PublisherFloat32)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt32, PublisherInt32)
			return;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported or invalid type passed to FROS2PublisherBarrier::AllocateNative, topic: "
			 "'%s'. Native will not be allocated."),
		*Topic);
}

FROS2Publisher* FROS2PublisherBarrier::GetNative()
{
	return Native.get();
}

const FROS2Publisher* FROS2PublisherBarrier::GetNative() const
{
	return Native.get();
}

EAGX_ROS2MessageType FROS2PublisherBarrier::GetMessageType() const
{
	return MessageType;
}

void FROS2PublisherBarrier::SendMessage(const FAGX_ROS2Message& Msg) const
{
	check(HasNative());

	switch (MessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_SEND_ROS2_MSGS(StdMsgs::FPublisherFloat32, FAGX_StdMsgsFloat32)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_SEND_ROS2_MSGS(StdMsgs::FPublisherInt32, FAGX_StdMsgsInt32)
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FROS2PublisherBarrier::SendMessage called on PublisherBarrier with an invalid "
			 "MessageType. The message will not be sent."));
}

void FROS2PublisherBarrier::ReleaseNative()
{
	Native = nullptr;
	MessageType = EAGX_ROS2MessageType::Invalid;
}
