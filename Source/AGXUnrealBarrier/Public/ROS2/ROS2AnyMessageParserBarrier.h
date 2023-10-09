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
	FROS2AnyMessageParserBarrier(FROS2AnyMessageParserBarrier&& Other);
	FROS2AnyMessageParserBarrier& operator=(FROS2AnyMessageParserBarrier&& Other);

	bool HasNative() const;

	void AllocateNative();

	FAnyMessageParser* GetNative();
	const FAnyMessageParser* GetNative() const;

	void ReleaseNative();

	void BeginParse(FAGX_AgxMsgsAny& Message);
	int8_t readInt8();

private:
	FROS2AnyMessageParserBarrier(const FROS2AnyMessageParserBarrier&) = delete;
	void operator=(const FROS2AnyMessageParserBarrier&) = delete;

private:
	std::unique_ptr<FAnyMessageParser> Native;
	std::unique_ptr<FAgxAny> Message;
};
