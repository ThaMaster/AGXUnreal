#pragma once

#include "ShapeBarrier.h"

#include <memory>

struct FShapeRef;
struct FSphereShapeRef;

class UWorld;

class AGXUNREALBARRIER_API FSphereShapeBarrier : public FShapeBarrier
{
public:
	FSphereShapeBarrier();
	virtual ~FSphereShapeBarrier() override;

	void SetRadius(float Radius, UWorld* World);
	float GetRadius(UWorld* World) const;

	FSphereShapeRef* GetNative();
	const FSphereShapeRef* GetNative() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FSphereShapeBarrier(const FSphereShapeBarrier&) = delete;
	void operator=(const FSphereShapeBarrier&) = delete;

private:
	std::unique_ptr<FSphereShapeRef> NativeRef;
};
