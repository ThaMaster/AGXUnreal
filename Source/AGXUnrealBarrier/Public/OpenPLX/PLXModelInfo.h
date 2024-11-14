// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <memory>

struct FPLXModelData;

class AGXUNREALBARRIER_API FPLXModelInfo
{
public:
	FPLXModelInfo();
	~FPLXModelInfo();

	bool HasNative() const;

private:
	FPLXModelInfo(const FPLXModelInfo&) = delete;
	void operator=(const FPLXModelInfo&) = delete;

	std::unique_ptr<FPLXModelData> Native;
};
