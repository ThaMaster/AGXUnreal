// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <cstdint>
#include <memory>

struct FAgxAny;
struct FAnyMessageParser;
struct FAGX_AgxMsgsAny;

class AGXUNREALBARRIER_API FROS2AnyMessageParserBarrier
{
public:
	FROS2AnyMessageParserBarrier();
	~FROS2AnyMessageParserBarrier();
	FROS2AnyMessageParserBarrier(FROS2AnyMessageParserBarrier&& Other) noexcept;
	FROS2AnyMessageParserBarrier& operator=(FROS2AnyMessageParserBarrier&& Other) noexcept;

	bool HasNative() const;

	void AllocateNative();

	FAnyMessageParser* GetNative();
	const FAnyMessageParser* GetNative() const;

	void ReleaseNative();

	void BeginParse(const FAGX_AgxMsgsAny& Message);
	int8_t ReadInt8();
	uint8_t ReadUInt8();
	int16_t ReadInt16();
	uint16_t ReadUInt16();
	int32_t ReadInt32();
	uint32_t ReadUInt32();
	int64_t ReadInt64();
	uint64_t ReadUInt64();
	float ReadFloat32();
	double ReadDouble64();
	FString ReadString();
	bool ReadBool();

	TArray<int8_t> ReadInt8Sequence();
	TArray<uint8_t> ReadUInt8Sequence();
	TArray<int16_t> ReadInt16Sequence();
	TArray<uint16_t> ReadUInt16Sequence();
	TArray<int32_t> ReadInt32Sequence();
	TArray<uint32_t> ReadUInt32Sequence();
	TArray<int64_t> ReadInt64Sequence();
	TArray<uint64_t> ReadUInt64Sequence();
	TArray<float> ReadFloat32Sequence();
	TArray<double> ReadDouble64Sequence();
	TArray<FString> ReadStringSequence();
	TArray<bool> ReadBoolSequence();

private:
	FROS2AnyMessageParserBarrier(const FROS2AnyMessageParserBarrier&) = delete;
	void operator=(const FROS2AnyMessageParserBarrier&) = delete;

private:
	std::unique_ptr<FAnyMessageParser> Native;
	std::unique_ptr<FAgxAny> Message;
};
