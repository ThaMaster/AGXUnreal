#pragma once

#include <memory>
#include "Math/Vector.h"
#include "Math/Quat.h"

struct FGeometryAndShapeRef;

class AGXUNREALBARRIER_API FShapeBarrier
{
public:
	FShapeBarrier();
	FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	virtual ~FShapeBarrier();

	bool HasNative() const;
	void AllocateNative();
	void ReleaseNative();
	FGeometryAndShapeRef* GetNative();
	const FGeometryAndShapeRef* GetNative() const;

	void SetLocalPosition(const FVector &Position);
	void SetLocalRotation(const FQuat &Rotation);

	FVector GetLocalPosition() const;
	FQuat GetLocalRotation() const;
	std::tuple<FVector, FQuat> GetLocalPositionAndRotation() const;

protected:
	FShapeBarrier(FShapeBarrier&& Other);

private:
	FShapeBarrier(const FShapeBarrier&) = delete;
	void operator=(const FShapeBarrier&) = delete;

private:
	/// \todo Are we allowed to have pure virtual classes in an Unreal plugin.
	///       Not allowed when inheriting from U/A classes, but we don't do that
	///       here.

	/**
	Called from AllocateNative. The subclass is responsible for creating the
	agxCollide::Shape instance, which is stored in NativeRef. FShapeBarrier
	creates the agxCollide::Geometry, in AllocateNative, and adds the
	agxCollide::Shape to it.
	*/
	virtual void AllocateNativeShape() = 0;
	virtual void ReleaseNativeShape() = 0;

protected:
	std::unique_ptr<FGeometryAndShapeRef> NativeRef;
};
