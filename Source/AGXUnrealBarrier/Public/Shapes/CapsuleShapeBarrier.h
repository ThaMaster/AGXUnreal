#pragma once

#include "Shapes/ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

class AGXUNREALBARRIER_API FCapsuleShapeBarrier : public FShapeBarrier
{
public:
	FCapsuleShapeBarrier();
	FCapsuleShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FCapsuleShapeBarrier(FCapsuleShapeBarrier&& Other);
	virtual ~FCapsuleShapeBarrier() override;

	void SetHeight(double Height);
	double GetHeight() const;

	void SetRadius(double Height);
	double GetRadius() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FCapsuleShapeBarrier(const FCapsuleShapeBarrier&) = delete;
	void operator=(const FCapsuleShapeBarrier&) = delete;
};
