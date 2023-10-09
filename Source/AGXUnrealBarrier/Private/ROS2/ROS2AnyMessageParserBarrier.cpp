// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/ROS2AnyMessageParserBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"

FROS2AnyMessageParserBarrier::FROS2AnyMessageParserBarrier()
{
}

FROS2AnyMessageParserBarrier::~FROS2AnyMessageParserBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration of the Native.
}

FROS2AnyMessageParserBarrier::FROS2AnyMessageParserBarrier(FROS2AnyMessageParserBarrier&& Other)
{
	*this = std::move(Other);
}

FROS2AnyMessageParserBarrier& FROS2AnyMessageParserBarrier::operator=(
	FROS2AnyMessageParserBarrier&& Other)
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	return *this;
}

bool FROS2AnyMessageParserBarrier::HasNative() const
{
	return Native != nullptr;
}

void FROS2AnyMessageParserBarrier::AllocateNative()
{
	check(!HasNative());
	Native = std::make_unique<FAnyMessageParser>(new agxIO::ROS2::AnyMessageParser());
}

FAnyMessageParser* FROS2AnyMessageParserBarrier::GetNative()
{
	return Native.get();
}

const FAnyMessageParser* FROS2AnyMessageParserBarrier::GetNative() const
{
	return Native.get();
}

void FROS2AnyMessageParserBarrier::ReleaseNative()
{
	Native = nullptr;
}

void FROS2AnyMessageParserBarrier::BeginParse(FAGX_AgxMsgsAny& InMessage)
{
	check(HasNative());

	// Here we store away a copy (of AGX type) of the message, used for subsequent reads.
	agxIO::ROS2::agxMsgs::Any* MessageAGX = new agxIO::ROS2::agxMsgs::Any();
	*MessageAGX = Convert(InMessage);
	Message = std::make_unique<FAgxAny>(MessageAGX);
	Native->Native->beginParse();
}

int8_t FROS2AnyMessageParserBarrier::readInt8()
{
	check(HasNative());
	if (Message == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Tried to read using a FROS2AnyMessageParserBarrier that does not have a reference "
				"to a Message. Ensure BeginParse has been called before calling this function."));
		return 0;
	}

	return Native->Native->readInt8(*Message->Native);
}
