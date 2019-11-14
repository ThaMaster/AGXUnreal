#pragma once

#include "Shapes/ShapeBarrier.h"

#include <memory>

class AGXUNREALBARRIER_API FHeightFieldShapeBarrier : public FShapeBarrier
{
public:
	FHeightFieldShapeBarrier();
	FHeightFieldShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FHeightFieldShapeBarrier(FHeightFieldShapeBarrier&& Other);
	virtual ~FHeightFieldShapeBarrier() override;

	void AllocateNative(int32 NumVerticesX, int32 NumVerticesY, float SizeX, float SizeY, const TArray<float>& Heights);

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

	void AllocateNativeHeightField(int32 NumVerticesX, int32 NumVerticesY, float SizeX, float SizeY, const TArray<float>& Heights);

private:
	FHeightFieldShapeBarrier(const FHeightFieldShapeBarrier&) = delete;
	void operator=(const FHeightFieldShapeBarrier&) = delete;
};
