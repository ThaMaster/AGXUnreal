#include "BoxShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

FBoxShapeBarrier::FBoxShapeBarrier()
	: FShapeBarrier()
	, NativeRef(new FBoxShapeRef)
{
}

FBoxShapeBarrier::~FBoxShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::uniqe_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FBoxShapeRef.
}

void FBoxShapeBarrier::SetHalfExtents(FVector HalfExtentsUnreal)
{
	check(HasNative());
	agx::Vec3 HalfExtentsAGX = Convert(HalfExtentsUnreal);
	NativeRef->Native->setHalfExtents(HalfExtentsAGX);
}

FVector FBoxShapeBarrier::GetHalfExtents() const
{
	check(HasNative());
	agx::Vec3 HalfExtentsAGX = NativeRef->Native->getHalfExtents();
	FVector HalfExtentsUnreal = Convert(HalfExtentsAGX);
	return HalfExtentsUnreal;
}

FBoxShapeRef* FBoxShapeBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

void FBoxShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->Native = new agxCollide::Box(agx::Vec3());
	NativeShapeRef->Native = NativeRef->Native;
}

void FBoxShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->Native = nullptr;
	NativeShapeRef->Native = nullptr;
}
