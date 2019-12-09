#pragma once

#include "Shapes/ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

class AGXUNREALBARRIER_API FCylinderShapeBarrier : public FShapeBarrier
{
public:
	FCylinderShapeBarrier();
	FCylinderShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FCylinderShapeBarrier(FCylinderShapeBarrier&& Other);
	virtual ~FCylinderShapeBarrier() override;

	void SetHeight(double Height);
	double GetHeight() const;

	void SetRadius(double Height);
	double GetRadius() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FCylinderShapeBarrier(const FCylinderShapeBarrier&) = delete;
	void operator=(const FCylinderShapeBarrier&) = delete;
};
