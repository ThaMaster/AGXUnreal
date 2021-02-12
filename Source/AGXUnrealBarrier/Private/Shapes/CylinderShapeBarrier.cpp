#include "Shapes/CylinderShapeBarrier.h"

#include "AGXRefs.h"

#include "BeginAGXIncludes.h"
#include <agxCollide/Cylinder.h>
#include "EndAGXIncludes.h"

#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

namespace
{
	agxCollide::Cylinder* NativeCylinder(FCylinderShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Cylinder>();
	}

	const agxCollide::Cylinder* NativeCylinder(const FCylinderShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Cylinder>();
	}
}

FCylinderShapeBarrier::FCylinderShapeBarrier()
	: FShapeBarrier()
{
}

FCylinderShapeBarrier::FCylinderShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Cylinder>());
}

FCylinderShapeBarrier::FCylinderShapeBarrier(FCylinderShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FCylinderShapeBarrier::~FCylinderShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FCylinderShapeRef.
}

void FCylinderShapeBarrier::SetHeight(double Height)
{
	check(HasNative());
	NativeCylinder(this)->setHeight(ConvertDistanceToAgx(Height));
}

double FCylinderShapeBarrier::GetHeight() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCylinder(this)->getHeight());
}

void FCylinderShapeBarrier::SetRadius(double Radius)
{
	check(HasNative());
	NativeCylinder(this)->setRadius(ConvertDistanceToAgx(Radius));
}

double FCylinderShapeBarrier::GetRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCylinder(this)->getRadius());
}

void FCylinderShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Cylinder(agx::Real(0.5), agx::Real(1.0));
}

void FCylinderShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
