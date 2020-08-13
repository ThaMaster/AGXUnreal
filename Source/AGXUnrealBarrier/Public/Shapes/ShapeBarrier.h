#pragma once

// AGXUnreal includes.
#include "Shapes/RenderData.h"

// Unreal Engine includes.
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>

struct FGeometryAndShapeRef;
class FShapeMaterialBarrier;

class AGXUNREALBARRIER_API FShapeBarrier
{
public:
	FShapeBarrier();
	FShapeBarrier(FShapeBarrier&& Other) noexcept;
	FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	virtual ~FShapeBarrier();

	FShapeBarrier& operator=(FShapeBarrier&& Other) noexcept;

	bool HasNative() const;
	void AllocateNative();
	void ReleaseNative();
	FGeometryAndShapeRef* GetNative();
	const FGeometryAndShapeRef* GetNative() const;

	template <typename T>
	T* GetNativeShape();

	template <typename T>
	const T* GetNativeShape() const;

	void SetLocalPosition(const FVector& Position);
	void SetLocalRotation(const FQuat& Rotation);

	FVector GetLocalPosition() const;
	FQuat GetLocalRotation() const;
	std::tuple<FVector, FQuat> GetLocalPositionAndRotation() const;

	void SetName(const FString& Name);
	FString GetName() const;

	void SetMaterial(const FShapeMaterialBarrier& Material);

	/// \todo Should GetMaterial() create a new FShapeMaterialBarrier, or get an existing somehow? If it
	/// creates a new FShapeMaterialBarrier we should implement comparison operators etc since multiple
	/// FShapeMaterialBarrier that points to the same native object should be logically seen as same
	/// object (similar to smart pointers).
	FShapeMaterialBarrier GetMaterial() const;

	void SetEnableCollisions(bool CanCollide);
	bool GetEnableCollisions() const;

	void AddCollisionGroup(const FName& GroupName);

	/**
	 * Get all collision groups registered for this Shape.
	 *
	 * AGX Dynamics supports both name- and integer-based IDs while AGXUnreal
	 * only supports named groups. Any integer ID found is converted to the
	 * string representation of that integer using FString::FromInt.
	 *
	 * \return A list of all collision groups registered for this shape.
	 */
	TArray<FName> GetCollisionGroups() const;

	FGuid GetGuid() const;

	bool HasRenderData() const;

	FAGX_RenderData GetRenderData() const;

protected:
	template <typename TFunc, typename... TPack>
	void AllocateNative(TFunc Factory, TPack... Params);

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
