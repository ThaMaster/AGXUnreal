// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "Shapes/ShapeBarrier.h"

#include <memory>

/**
 * A ShapeBarrier that represents a Shape that we don't know what shape it has. Used when we
 * extract Geometries from AGX Dynamics from e.g., contacts.
 */
class AGXUNREALBARRIER_API FEmptyShapeBarrier : public FShapeBarrier
{
public:
	FEmptyShapeBarrier();
	FEmptyShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FEmptyShapeBarrier(FEmptyShapeBarrier&& Other);
	virtual ~FEmptyShapeBarrier() override;

	virtual bool HasNative() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FEmptyShapeBarrier(const FEmptyShapeBarrier&) = delete;
	void operator=(const FEmptyShapeBarrier&) = delete;
};
