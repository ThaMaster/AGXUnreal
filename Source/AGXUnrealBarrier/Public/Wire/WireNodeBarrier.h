#pragma once

// Unreal Engine includes.
#include "Math/Vector.h"

// System includes.
#include <memory>

struct FWireNodeRef;
class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FWireNodeBarrier
{
public:
	FWireNodeBarrier();
	FWireNodeBarrier(FWireNodeBarrier&& Other);
	FWireNodeBarrier(std::unique_ptr<FWireNodeRef>&& Native);
	~FWireNodeBarrier();

	bool HasNative() const;
	void AllocateNativeFreeNode(const FVector& WorldLocation);
	void AllocateNativeEyeNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	void AllocateNativeBodyFixedNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	FWireNodeRef* GetNative();
	const FWireNodeRef* GetNative() const;
	void ReleaseNative();

private:
	FWireNodeBarrier(const FWireNodeBarrier&) = delete;
	void operator=(const FWireNodeBarrier&) = delete;

private:
	std::unique_ptr<FWireNodeRef> NativeRef;
};

