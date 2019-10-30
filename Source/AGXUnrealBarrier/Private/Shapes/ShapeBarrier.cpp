#include "Shapes/ShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

FShapeBarrier::FShapeBarrier()
	: NativeRef{new FGeometryAndShapeRef}
{
}

FShapeBarrier::FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: NativeRef{std::move(Native)}
{
}

FShapeBarrier::FShapeBarrier(FShapeBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))

{
}

FShapeBarrier::~FShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FGeometryAndShapeRef.
}

bool FShapeBarrier::HasNative() const
{
	return NativeRef->NativeGeometry != nullptr && NativeRef->NativeShape != nullptr;
}

void FShapeBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->NativeGeometry = new agxCollide::Geometry();
	AllocateNativeShape();
	NativeRef->NativeGeometry->add(NativeRef->NativeShape);
}

void FShapeBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->NativeGeometry->remove(NativeRef->NativeShape);
	ReleaseNativeShape();
	NativeRef->NativeGeometry = nullptr;
}

FGeometryAndShapeRef* FShapeBarrier::GetNative()
{
	return NativeRef.get();
}

const FGeometryAndShapeRef* FShapeBarrier::GetNative() const
{
	return NativeRef.get();
}

void FShapeBarrier::SetLocalPosition(const FVector &Position, UWorld* World)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalPosition(ConvertDistance(Position, World));
}

void FShapeBarrier::SetLocalRotation(const FQuat &Rotation)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalRotation(Convert(Rotation));
}
