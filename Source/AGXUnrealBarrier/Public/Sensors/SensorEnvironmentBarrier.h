// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FSensorEnvironmentRef;

class AGXUNREALBARRIER_API FSensorEnvironmentBarrier
{
public:
	FSensorEnvironmentBarrier();
	FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native);
	FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other);
	~FSensorEnvironmentBarrier();

	bool HasNative() const;
	void AllocateNative();
	FSensorEnvironmentRef* GetNative();
	const FSensorEnvironmentRef* GetNative() const;

	void ReleaseNative();

private:
	FSensorEnvironmentBarrier(const FSensorEnvironmentBarrier&) = delete;
	void operator=(const FSensorEnvironmentBarrier&) = delete;

private:
	std::unique_ptr<FSensorEnvironmentRef> NativeRef;
};
