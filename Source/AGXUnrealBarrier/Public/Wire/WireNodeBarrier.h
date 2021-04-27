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
	FWireNodeBarrier(const FWireNodeBarrier& InOther);
	FWireNodeBarrier(FWireNodeBarrier&& InOther);
	FWireNodeBarrier(std::unique_ptr<FWireNodeRef>&& InNative);
	~FWireNodeBarrier();

	FWireNodeBarrier& operator=(const FWireNodeBarrier& InOther);

	bool HasNative() const;
	void AllocateNativeFreeNode(const FVector& WorldLocation);
	void AllocateNativeEyeNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	void AllocateNativeBodyFixedNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	FWireNodeRef* GetNative();
	const FWireNodeRef* GetNative() const;
	void ReleaseNative();

	FVector GetWorldLocation() const;

private:
	std::unique_ptr<FWireNodeRef> NativeRef;
};

