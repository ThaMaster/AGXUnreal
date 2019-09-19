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
	FGeometryRef* GetNativeGeometry();
	FShapeRef* GetNativeShape();

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

protected:
	std::unique_ptr<FGeometryRef> NativeGeometryRef;
	std::unique_ptr<FShapeRef> NativeShapeRef;
	// NativeRef is owned by the subclasses of FShapeBarrier, e.g., FBoxBarrier.
};
