#include "Shapes/ShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

FShapeBarrier::FShapeBarrier()
	: NativeRef {new FGeometryAndShapeRef}
{
}

FShapeBarrier::FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: NativeRef {std::move(Native)}
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

void FShapeBarrier::SetLocalPosition(const FVector& Position, UWorld* World)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalPosition(ConvertVector(Position, World));
}

void FShapeBarrier::SetLocalRotation(const FQuat& Rotation)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalRotation(Convert(Rotation));
}

FVector FShapeBarrier::GetLocalPosition(UWorld* World) const
{
	return std::get<0>(GetLocalPositionAndRotation(World));
}

FQuat FShapeBarrier::GetLocalRotation(UWorld* World) const
{
	return std::get<1>(GetLocalPositionAndRotation(World));
}

namespace
{
	agxCollide::ShapeIterator FindShape(agxCollide::Geometry* Geometry, agxCollide::Shape* Shape)
	{
		agxCollide::ShapeIterator Iterator(Geometry);
		while (Iterator.isValid() && Iterator.getShape() != Shape)
		{
			Iterator.next();
		}
		if (!Iterator.isValid())
		{
			UE_LOG(LogTemp, Error,
				TEXT("Found invalid FShapeBarrier. The native Geometry does not contain the native Shape."));
		}
		return Iterator;
	}
}

std::tuple<FVector, FQuat> FShapeBarrier::GetLocalPositionAndRotation(UWorld* World) const
{
	check(HasNative());
	agxCollide::ShapeIterator Iterator = FindShape(NativeRef->NativeGeometry, NativeRef->NativeShape);
	if (!Iterator.isValid())
	{
		return {FVector::ZeroVector, FQuat::Identity};
	}

	// The ShapeTransform is always Identity when the Native objects have been
	// created from AGXUnreal objects. However, it can be a non-Identity transform
	// during import from a .agx archive. Split this implementation if this step
	// is shown to be a performance problem.
	const agx::AffineMatrix4x4& GeometryTransform = NativeRef->NativeGeometry->getLocalTransform();
	const agx::AffineMatrix4x4& ShapeTransform = Iterator.getLocalTransform();
	const agx::AffineMatrix4x4 ShapeRelativeBody = ShapeTransform * GeometryTransform;
	return {ConvertVector(ShapeRelativeBody.getTranslate(), World), Convert(ShapeRelativeBody.getRotate())};
}
