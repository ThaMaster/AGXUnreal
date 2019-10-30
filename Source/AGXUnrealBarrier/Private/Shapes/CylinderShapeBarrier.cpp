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

void FCylinderShapeBarrier::SetHeight(double Height, UWorld* World)
{
	check(HasNative());
	NativeCylinder(this)->setHeight(ConvertDistanceToAgx(Height, World));
}

double FCylinderShapeBarrier::GetHeight(UWorld* World) const
{
	check(HasNative());
	return ConvertDistanceToUnrealD(NativeCylinder(this)->getHeight(), World);
}

void FCylinderShapeBarrier::SetRadius(double Radius, UWorld* World)
{
	check(HasNative());
	NativeCylinder(this)->setRadius(ConvertDistanceToAgx(Radius, World));
}

double FCylinderShapeBarrier::GetRadius(UWorld* World) const
{
	check(HasNative());
	return ConvertDistanceToUnrealD(NativeCylinder(this)->getRadius(), World);
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
