#include "ShovelBarrier.h"

// AGXUnrealBarrier includes.
#include "AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "agxTerrain/Shovel.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "Math/TwoVectors.h"
#include "Math/Vector.h"

FShovelBarrier::FShovelBarrier()
	: NativeRef {new FShovelRef}
{
}

FShovelBarrier::FShovelBarrier(std::unique_ptr<FShovelRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FShovelBarrier::FShovelBarrier(FShovelBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FShovelBarrier::~FShovelBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the
	// definition, not just the forward declaration, of FShovelRef.
}

bool FShovelBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FShovelBarrier::AllocateNative(
	FRigidBodyBarrier& Body, const FTwoVectors& TopEdge, const FTwoVectors& CuttingEdge,
	const FVector& CuttingDirection)
{
	check(!HasNative());
	agx::RigidBody* BodyAGX = Body.GetNative()->Native;
	agx::Line TopEdgeAGX = ConvertDistance(TopEdge);
	agx::Line CuttingEdgeAGX = ConvertDistance(CuttingEdge);
	agx::Vec3 CuttingDirectionAGX = ConvertDistance(CuttingDirection);
	NativeRef->Native = new agxTerrain::Shovel(BodyAGX, TopEdgeAGX, CuttingEdgeAGX, CuttingDirectionAGX);
}

FShovelRef* FShovelBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FShovelRef* FShovelBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FShovelBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
