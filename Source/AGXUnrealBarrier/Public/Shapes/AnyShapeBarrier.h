// Copyright 2023, Algoryx Simulation AB

#pragma once

#include "Shapes/ShapeBarrier.h"

/**
 * A Shape Barrier that may wrap any AGX Dynamics shape type.
 *
 * Cannot be used to allocate new AGX Dynamics shapes, since it doesn't know what type of shape to
 * allocate.
 */
class AGXUNREALBARRIER_API FAnyShapeBarrier : public FShapeBarrier
{
public:
	FAnyShapeBarrier();
	FAnyShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FAnyShapeBarrier(FAnyShapeBarrier&& Other);
	virtual ~FAnyShapeBarrier() override;

private:
	/**
	 * Will always fail with an error message.
	 */
	virtual void AllocateNativeShape() override;

	/**
	 * Will always fail with an error message.
	 */
	virtual void ReleaseNativeShape() override;

private:
	FAnyShapeBarrier(const FAnyShapeBarrier&) = delete;
	void operator=(const FAnyShapeBarrier&) = delete;
};
