// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtShapeInstance;
class FRtShapeBarrier;
class FSensorEnvironmentBarrier;

class AGXUNREALBARRIER_API FRtShapeInstanceBarrier
{
public:
	FRtShapeInstanceBarrier();
	FRtShapeInstanceBarrier(std::unique_ptr<FRtShapeInstance> Native);
	FRtShapeInstanceBarrier(FRtShapeInstanceBarrier&& Other);
	~FRtShapeInstanceBarrier();

	bool HasNative() const;
	void AllocateNative(FRtShapeBarrier& Shape, FSensorEnvironmentBarrier& Environment, float Reflectivity);
	FRtShapeInstance* GetNative();
	const FRtShapeInstance* GetNative() const;

	void SetTransform(const FTransform& Transform);

private:
	FRtShapeInstanceBarrier(const FRtShapeInstanceBarrier&) = delete;
	void operator=(const FRtShapeInstanceBarrier&) = delete;

private:
	std::unique_ptr<FRtShapeInstance> NativeRef; 
};
