#pragma once

#include <memory>
#include "Math/Vector.h"
#include "Math/Quat.h"

class FMaterialBarrier;
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

	void SetLocalPosition(const FVector &Position, UWorld* World);
	void SetLocalRotation(const FQuat &Rotation);

	FVector GetLocalPosition(UWorld* World) const;
	FQuat GetLocalRotation(UWorld* World) const;
	std::tuple<FVector, FQuat> GetLocalPositionAndRotation(UWorld* World) const;

	void SetMaterial(const FMaterialBarrier& Material);
	/// \todo Should GetMaterial() create a new FMaterialBarrier, or get an existing somehow? If it creates a new
	/// FMaterialBarrier we should implement comparison operators etc since multiple FMaterialBarrier that points
	/// to the same native object should be logically seen as same object (similar to smart pointers).

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
