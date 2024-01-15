// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "Shapes/ShapeBarrier.h"

#include <memory>

class AGXUNREALBARRIER_API FSphereShapeBarrier : public FShapeBarrier
{
public:
	FSphereShapeBarrier();
	FSphereShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FSphereShapeBarrier(FSphereShapeBarrier&& Other);
	virtual ~FSphereShapeBarrier() override;

	void SetRadius(float Radius);
	float GetRadius() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FSphereShapeBarrier(const FSphereShapeBarrier&) = delete;
	void operator=(const FSphereShapeBarrier&) = delete;
};
