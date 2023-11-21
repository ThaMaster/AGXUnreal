// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct FAGX_AgxMsgsAny;
struct FAnyMessageBuilder;

class AGXUNREALBARRIER_API FROS2AnyMessageBuilderBarrier
{
public:
	FROS2AnyMessageBuilderBarrier();
	~FROS2AnyMessageBuilderBarrier();
	FROS2AnyMessageBuilderBarrier(FROS2AnyMessageBuilderBarrier&& Other) noexcept;
	FROS2AnyMessageBuilderBarrier& operator=(FROS2AnyMessageBuilderBarrier&& Other) noexcept;

	bool HasNative() const;

	void AllocateNative();

	FAnyMessageBuilder* GetNative();
	const FAnyMessageBuilder* GetNative() const;

	void ReleaseNative();

	void BeginMessage();

	void WriteInt8(int8_t d);
	void WriteUInt8(uint8_t d);
	void WriteInt16(int16_t d);
	void WriteUInt16(uint16_t d);
	void WriteInt32(int32_t d);
	void WriteUInt32(uint32_t d);
	void WriteInt64(int64_t d);
	void WriteUInt64(uint64_t d);
	void WriteFloat32(float d);
	void WriteDouble64(double d);
	void WriteString(const FString& d);
	void WriteBool(bool d);

	void WriteInt8Sequence(const TArray<int8_t>& d);
	void WriteUInt8Sequence(const TArray<uint8_t>& d);
	void WriteInt16Sequence(const TArray<int16_t>& d);
	void WriteUInt16Sequence(const TArray<uint16_t>& d);
	void WriteInt32Sequence(const TArray<int32_t>& d);
	void WriteUInt32Sequence(const TArray<uint32_t>& d);
	void WriteInt64Sequence(const TArray<int64_t>& d);
	void WriteUInt64Sequence(const TArray<uint64_t>& d);
	void WriteFloat32Sequence(const TArray<float>& d);
	void WriteDouble64Sequence(const TArray<double>& d);
	void WriteStringSequence(const TArray<FString>& d);
	void WriteBoolSequence(const TArray<bool>& d);

	FAGX_AgxMsgsAny GetBuiltMessage() const;

private:
	FROS2AnyMessageBuilderBarrier(const FROS2AnyMessageBuilderBarrier&) = delete;
	void operator=(const FROS2AnyMessageBuilderBarrier&) = delete;

private:
	std::unique_ptr<FAnyMessageBuilder> Native;
};
