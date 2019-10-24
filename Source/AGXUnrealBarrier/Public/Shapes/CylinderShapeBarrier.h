#pragma once


#include "Shapes/ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

class UWorld;

class AGXUNREALBARRIER_API FCylinderShapeBarrier : public FShapeBarrier
{
public:
	FCylinderShapeBarrier();
	FCylinderShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FCylinderShapeBarrier(FCylinderShapeBarrier&& Other);
	virtual ~FCylinderShapeBarrier() override;

	void SetHeight(double Height, UWorld* World);
	double GetHeight(UWorld* World) const;

	void SetRadius(double Height, UWorld* World);
	double GetRadius(UWorld* World) const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FCylinderShapeBarrier(const FCylinderShapeBarrier&) = delete;
	void operator=(const FCylinderShapeBarrier&) = delete;
};
