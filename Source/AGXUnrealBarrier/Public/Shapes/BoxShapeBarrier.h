// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "Shapes/ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

class AGXUNREALBARRIER_API FBoxShapeBarrier : public FShapeBarrier
{
public:
	FBoxShapeBarrier();
	FBoxShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FBoxShapeBarrier(FBoxShapeBarrier&& Other);
	virtual ~FBoxShapeBarrier() override;

	void SetHalfExtents(const FVector& NewHalfExtents);
	FVector GetHalfExtents() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FBoxShapeBarrier(const FBoxShapeBarrier&) = delete;
	void operator=(const FBoxShapeBarrier&) = delete;
};
