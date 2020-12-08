#include "Shapes/SphereShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "TypeConversions.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Sphere.h>
#include "EndAGXIncludes.h"

namespace
{
	agxCollide::Sphere* NativeSphere(FSphereShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Sphere>();
	}

	const agxCollide::Sphere* NativeSphere(const FSphereShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Sphere>();
	}
}

FSphereShapeBarrier::FSphereShapeBarrier()
	: FShapeBarrier()
{
}

FSphereShapeBarrier::FSphereShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(GetNative()->NativeShape->is<agxCollide::Sphere>());
}

FSphereShapeBarrier::FSphereShapeBarrier(FSphereShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FSphereShapeBarrier::~FSphereShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FSphereShapeRef.
}

void FSphereShapeBarrier::SetRadius(float RadiusUnreal)
{
	check(HasNative());
	agx::Real RadiusAGX = ConvertDistance(RadiusUnreal);
	NativeSphere(this)->setRadius(RadiusAGX);
}

float FSphereShapeBarrier::GetRadius() const
{
	check(HasNative());
	agx::Real RadiusAGX = NativeSphere(this)->getRadius();
	float RadiusUnreal = ConvertDistance(RadiusAGX);
	return RadiusUnreal;
}

void FSphereShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Sphere(agx::Real());
}

void FSphereShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
