// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/ROS2AnyMessageBuilderBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"


FROS2AnyMessageBuilderBarrier::FROS2AnyMessageBuilderBarrier()
{
}

FROS2AnyMessageBuilderBarrier::~FROS2AnyMessageBuilderBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration of the Native.
}

FROS2AnyMessageBuilderBarrier::FROS2AnyMessageBuilderBarrier(FROS2AnyMessageBuilderBarrier&& Other)
{
	*this = std::move(Other);
}

FROS2AnyMessageBuilderBarrier& FROS2AnyMessageBuilderBarrier::operator=(FROS2AnyMessageBuilderBarrier&& Other)
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	return *this;
}

bool FROS2AnyMessageBuilderBarrier::HasNative() const
{
	return Native != nullptr;
}

void FROS2AnyMessageBuilderBarrier::AllocateNative()
{
	check(!HasNative());
	Native = std::make_unique<FAnyMessageBuilder>(new agxIO::ROS2::AnyMessageBuilder());
}

FAnyMessageBuilder* FROS2AnyMessageBuilderBarrier::GetNative()
{
	return Native.get();
}

const FAnyMessageBuilder* FROS2AnyMessageBuilderBarrier::GetNative() const
{
	return Native.get();
}

void FROS2AnyMessageBuilderBarrier::ReleaseNative()
{
	Native = nullptr;
}

void FROS2AnyMessageBuilderBarrier::BeginMessage()
{
	check(HasNative());
	Native->Native->beginMessage();
}

void FROS2AnyMessageBuilderBarrier::WriteInt8(int8_t d)
{
	check(HasNative());
	Native->Native->writeInt8(d);
}

FAGX_AgxMsgsAny FROS2AnyMessageBuilderBarrier::GetMessage()
{
	return Convert(Native->Native->getMessage());
}
