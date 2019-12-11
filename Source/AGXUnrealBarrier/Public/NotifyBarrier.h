#pragma once

#include "CoreMinimal.h"
#include <LogVerbosity.h>

#include <memory>

struct FNotifyRef;

class FNotifyBarrier
{
public:
	FNotifyBarrier();
	FNotifyBarrier(std::unique_ptr<FNotifyRef> Native);
	FNotifyBarrier(FNotifyBarrier&& Other);
	~FNotifyBarrier();

	bool HasNative() const;
	void AllocateNative();
	FNotifyRef* GetNative();
	const FNotifyRef* GetNative() const;

	void StartAgxNotify(ELogVerbosity::Type LogVerbosity);
	void StopAgxNotify();

private:
	FNotifyBarrier(const FNotifyBarrier&) = delete;
	void operator=(const FNotifyBarrier&) = delete;

private:
	std::unique_ptr<FNotifyRef> NativeRef;
};
