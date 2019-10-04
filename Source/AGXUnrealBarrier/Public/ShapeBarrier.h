#pragma once

#include <memory>

struct FGeometryRef;
struct FShapeRef;

class AGXUNREALBARRIER_API FShapeBarrier
{
public:
	FShapeBarrier();
	virtual ~FShapeBarrier();

	bool HasNative() const;
	void AllocateNative();
	void ReleaseNative();
	FGeometryRef* GetNativeGeometry();
	FShapeRef* GetNativeShape();

	void SetLocalPosition(const FVector &Position, UWorld* World);
	void SetLocalRotation(const FQuat &Rotation);

private:
	FShapeBarrier(const FShapeBarrier&) = delete;
	void operator=(const FShapeBarrier&) = delete;

private:
	/// \todo Are we allowed to have pure virtual classes in an Unreal plugin.
	///       Not allowed when inheriting from U/A classes, but we don't do that
	///       here.

	/**
	Called from AllocateNative. The subclass is responsible for creating the
	agxCollide::Shape instance, which is stored in NativeShapeRef. FShapeBarrier
	creates the agxCollide::Geometry, in AllocateNative, and adds the
	agxCollide::Shape to it.

	\return The FShapeRef the subclass decide the actual type of.
	*/
	virtual void AllocateNativeShape() = 0;
	virtual void ReleaseNativeShape() = 0;

protected:
	std::unique_ptr<FGeometryRef> NativeGeometryRef;
	std::unique_ptr<FShapeRef> NativeShapeRef;
	// NativeRef is held by the subclasses of FShapeBarrier, e.g., FBoxBarrier.
	// The NativeRef and the NativeShapeRef should always point to the same
	// object.
};
