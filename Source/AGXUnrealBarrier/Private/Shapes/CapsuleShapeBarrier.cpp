// Copyright 2022, Algoryx Simulation AB.


#include "Shapes/CapsuleShapeBarrier.h"

#include "AGXRefs.h"

#include "BeginAGXIncludes.h"
#include <agxCollide/Capsule.h>
#include "EndAGXIncludes.h"

#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

namespace
{
	agxCollide::Capsule* NativeCapsule(FCapsuleShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Capsule>();
	}

	const agxCollide::Capsule* NativeCapsule(const FCapsuleShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Capsule>();
	}
}

FCapsuleShapeBarrier::FCapsuleShapeBarrier()
	: FShapeBarrier()
{
}

FCapsuleShapeBarrier::FCapsuleShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Capsule>());
}

FCapsuleShapeBarrier::FCapsuleShapeBarrier(FCapsuleShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FCapsuleShapeBarrier::~FCapsuleShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FCapsuleShapeRef.
}

void FCapsuleShapeBarrier::SetHeight(double Height)
{
	check(HasNative());
	NativeCapsule(this)->setHeight(ConvertDistanceToAgx(Height));
}

double FCapsuleShapeBarrier::GetHeight() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCapsule(this)->getHeight());
}

void FCapsuleShapeBarrier::SetRadius(double Radius)
{
	check(HasNative());
	NativeCapsule(this)->setRadius(ConvertDistanceToAgx(Radius));
}

double FCapsuleShapeBarrier::GetRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCapsule(this)->getRadius());
}

void FCapsuleShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Capsule(agx::Real(0.5), agx::Real(1.0));
}

void FCapsuleShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
