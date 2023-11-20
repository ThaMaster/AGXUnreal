// Copyright 2023, Algoryx Simulation AB.

#include "ROS2/ROS2AnyMessageBuilderBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"
#include "TypeConversions.h"


FROS2AnyMessageBuilderBarrier::FROS2AnyMessageBuilderBarrier()
{
}

FROS2AnyMessageBuilderBarrier::~FROS2AnyMessageBuilderBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration of the Native.
}

FROS2AnyMessageBuilderBarrier::FROS2AnyMessageBuilderBarrier(
	FROS2AnyMessageBuilderBarrier&& Other) noexcept
{
	*this = std::move(Other);
}

FROS2AnyMessageBuilderBarrier& FROS2AnyMessageBuilderBarrier::operator=(
	FROS2AnyMessageBuilderBarrier&& Other) noexcept
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	return *this;
}

bool FROS2AnyMessageBuilderBarrier::HasNative() const
{
	return Native != nullptr && Native->Native != nullptr;
}

void FROS2AnyMessageBuilderBarrier::AllocateNative()
{
	check(!HasNative());
	Native = std::make_unique<FAnyMessageBuilder>(new agxROS2::AnyMessageBuilder());
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

void FROS2AnyMessageBuilderBarrier::WriteUInt8(uint8_t d)
{
	check(HasNative());
	Native->Native->writeUInt8(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt16(int16_t d)
{
	check(HasNative());
	Native->Native->writeInt16(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt16(uint16_t d)
{
	check(HasNative());
	Native->Native->writeUInt16(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt32(int32_t d)
{
	check(HasNative());
	Native->Native->writeInt32(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt32(uint32_t d)
{
	check(HasNative());
	Native->Native->writeUInt32(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt64(int64_t d)
{
	check(HasNative());
	Native->Native->writeInt64(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt64(uint64_t d)
{
	check(HasNative());
	Native->Native->writeUInt64(d);
}

void FROS2AnyMessageBuilderBarrier::WriteFloat32(float d)
{
	check(HasNative());
	Native->Native->writeFloat32(d);
}

void FROS2AnyMessageBuilderBarrier::WriteDouble64(double d)
{
	check(HasNative());
	Native->Native->writeDouble64(d);
}

void FROS2AnyMessageBuilderBarrier::WriteString(const FString& d)
{
	check(HasNative());
	Native->Native->writeString(Convert(d));
}

void FROS2AnyMessageBuilderBarrier::WriteBool(bool d)
{
	check(HasNative());
	Native->Native->writeBool(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt8Sequence(const TArray<int8_t>& d)
{
	check(HasNative());
	Native->Native->writeInt8Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt8Sequence(const TArray<uint8_t>& d)
{
	check(HasNative());
	Native->Native->writeUInt8Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteInt16Sequence(const TArray<int16_t>& d)
{
	check(HasNative());
	Native->Native->writeInt16Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt16Sequence(const TArray<uint16_t>& d)
{
	check(HasNative());
	Native->Native->writeUInt16Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteInt32Sequence(const TArray<int32_t>& d)
{
	check(HasNative());
	Native->Native->writeInt32Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt32Sequence(const TArray<uint32_t>& d)
{
	check(HasNative());
	Native->Native->writeUInt32Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteInt64Sequence(const TArray<int64_t>& d)
{
	check(HasNative());
	Native->Native->writeInt64Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt64Sequence(const TArray<uint64_t>& d)
{
	check(HasNative());
	Native->Native->writeUInt64Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteFloat32Sequence(const TArray<float>& d)
{
	check(HasNative());
	Native->Native->writeFloat32Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteDouble64Sequence(const TArray<double>& d)
{
	check(HasNative());
	Native->Native->writeDouble64Sequence(ToStdArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteStringSequence(const TArray<FString>& d)
{
	check(HasNative());
	Native->Native->writeStringSequence(ToStdStringArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteBoolSequence(const TArray<bool>& d)
{
	check(HasNative());
	Native->Native->writeBoolSequence(ToStdArray(d));
}

FAGX_AgxMsgsAny FROS2AnyMessageBuilderBarrier::GetMessage() const
{
	return Convert(Native->Native->getMessage());
}
