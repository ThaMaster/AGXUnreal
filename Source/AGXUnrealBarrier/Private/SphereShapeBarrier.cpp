#include "SphereShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

FSphereShapeBarrier::FSphereShapeBarrier()
	: FShapeBarrier()
	, NativeRef{new FSphereShapeRef}
{
}

FSphereShapeBarrier::~FSphereShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::uniqe_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FSphereShapeRef.
}

void FSphereShapeBarrier::SetRadius(float RadiusUnreal, UWorld* World)
{
	check(HasNative());
	agx::Real RadiusAGX = ConvertDistance(RadiusUnreal, World);
	NativeRef->Native->setRadius(RadiusAGX);
}

float FSphereShapeBarrier::GetRadius(UWorld* World) const
{
	check(HasNative());
	agx::Real RadiusAGX = NativeRef->Native->getRadius();
	float RadiusUnreal = ConvertDistance(RadiusAGX, World);
	return RadiusUnreal;
}

FSphereShapeRef* FSphereShapeBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FSphereShapeRef* FSphereShapeBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FSphereShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->Native = new agxCollide::Sphere(agx::Real());
	NativeShapeRef->Native = NativeRef->Native;
}

void FSphereShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->Native = nullptr;
	NativeShapeRef->Native = nullptr;
}
