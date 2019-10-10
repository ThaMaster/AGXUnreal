#pragma once

#include "ShapeBarrier.h"

#include <memory>

class UWorld;

class AGXUNREALBARRIER_API FSphereShapeBarrier : public FShapeBarrier
{
public:
	FSphereShapeBarrier();
	FSphereShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FSphereShapeBarrier(FSphereShapeBarrier&& Other);
	virtual ~FSphereShapeBarrier() override;

	void SetRadius(float Radius, UWorld* World);
	float GetRadius(UWorld* World) const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FSphereShapeBarrier(const FSphereShapeBarrier&) = delete;
	void operator=(const FSphereShapeBarrier&) = delete;
};
