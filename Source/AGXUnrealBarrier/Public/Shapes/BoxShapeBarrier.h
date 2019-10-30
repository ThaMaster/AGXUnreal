#pragma once


#include "Shapes/ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

class UWorld;

class AGXUNREALBARRIER_API FBoxShapeBarrier : public FShapeBarrier
{
public:
	FBoxShapeBarrier();
	FBoxShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FBoxShapeBarrier(FBoxShapeBarrier&& Other);
	virtual ~FBoxShapeBarrier() override;

	void SetHalfExtents(FVector NewHalfExtents, UWorld* World);
	FVector GetHalfExtents(UWorld* World) const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FBoxShapeBarrier(const FBoxShapeBarrier&) = delete;
	void operator=(const FBoxShapeBarrier&) = delete;
};
