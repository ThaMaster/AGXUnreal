// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <cstdint>
#include <memory>

struct FAGX_AgxMsgsAny;
struct FAnyMessageBuilder;

class AGXUNREALBARRIER_API FROS2AnyMessageBuilderBarrier
{
public:
	FROS2AnyMessageBuilderBarrier();
	~FROS2AnyMessageBuilderBarrier();
	FROS2AnyMessageBuilderBarrier(FROS2AnyMessageBuilderBarrier&& Other);
	FROS2AnyMessageBuilderBarrier& operator=(FROS2AnyMessageBuilderBarrier&& Other);

	bool HasNative() const;

	void AllocateNative();

	FAnyMessageBuilder* GetNative();
	const FAnyMessageBuilder* GetNative() const;

	void ReleaseNative();

	void BeginMessage();
	void WriteInt8(int8_t d);

	FAGX_AgxMsgsAny GetMessage();

private:
	FROS2AnyMessageBuilderBarrier(const FROS2AnyMessageBuilderBarrier&) = delete;
	void operator=(const FROS2AnyMessageBuilderBarrier&) = delete;

private:
	std::unique_ptr<FAnyMessageBuilder> Native;
};
