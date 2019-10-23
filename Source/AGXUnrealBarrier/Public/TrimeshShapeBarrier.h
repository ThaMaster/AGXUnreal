#pragma once


#include "ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

struct FTriIndices;
class UWorld;

class AGXUNREALBARRIER_API FTrimeshShapeBarrier : public FShapeBarrier
{
public:
	FTrimeshShapeBarrier();
	FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FTrimeshShapeBarrier(FTrimeshShapeBarrier&& Other);
	virtual ~FTrimeshShapeBarrier() override;
	
	void AllocateNative(const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise, UWorld* World);

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FTrimeshShapeBarrier(const FTrimeshShapeBarrier&) = delete;
	void operator=(const FTrimeshShapeBarrier&) = delete;

	struct AllocationParameters
	{
		const TArray<FVector>* Vertices;
		const TArray<FTriIndices>* TriIndices;
		bool bClockwise;
		UWorld* World;
	};

	std::weak_ptr<AllocationParameters> TemporaryAllocationParameters;
};
