#include "Wire/WireBarrier.h"

// AGX Unreal includes.
#include "TypeConversions.h"
#include "Wire/WireNodeBarrier.h"
#include "Wire/WireNodeRef.h"
#include "Wire/WireRef.h"
#include "Wire/WireWinchBarrier.h"
#include "Wire/WireWinchRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Material.h>
#include "EndAGXIncludes.h"

FWireBarrier::FWireBarrier()
	: NativeRef {new FWireRef()}
{
}

FWireBarrier::FWireBarrier(FWireBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset();
}

FWireBarrier::FWireBarrier(std::unique_ptr<FWireRef>&& Native)
	: NativeRef {std::move(Native)}
{
	Native.reset();
}

FWireBarrier::~FWireBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FWireRef.
}

bool FWireBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

/** Damping and Young's modulus for demonstration/experimentation purposes. Will be replaced
 * with Wire Material shortly. */
void FWireBarrier::AllocateNative(
	float Radius, float ResolutionPerUnitLength, float DampingBend, float DampingStretch,
	float YoungsModulusBend, float YoungsModulusStretch)
{
	check(!HasNative());
	agx::Real RadiusAGX = ConvertDistance(Radius);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInv(ResolutionPerUnitLength);
	NativeRef->Native = new agxWire::Wire(RadiusAGX, ResolutionPerUnitLengthAGX);

/// @todo REMOVE THIS!
/// This is only for testing.
/// Add proper Wire Material support.
#if 1
	static int Counter = 0;
	++Counter;
	agx::MaterialRef Material = new agx::Material(agx::String::format("WireMaterial_%d", Counter));
	agx::WireMaterial* WireMaterial = Material->getWireMaterial();
	WireMaterial->setDampingBend(DampingBend);
	WireMaterial->setDampingStretch(DampingStretch);
	WireMaterial->setYoungsModulusBend(YoungsModulusBend);
	WireMaterial->setYoungsModulusStretch(YoungsModulusStretch);
	NativeRef->Native->setMaterial(Material);
#endif
}

FWireRef* FWireBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FWireRef* FWireBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FWireBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FWireBarrier::SetScaleConstant(double ScaleConstant)
{
	check(HasNative());
	NativeRef->Native->getParameterController()->setScaleConstant(ScaleConstant);
}

double FWireBarrier::GetScaleConstant() const
{
	check(HasNative());
	return NativeRef->Native->getParameterController()->getScaleConstant();
}

void FWireBarrier::SetLinearVelocityDamping(double Damping)
{
	check(HasNative());
	NativeRef->Native->setLinearVelocityDamping(Damping);
}

double FWireBarrier::GetLinearVelocityDamping() const
{
	check(HasNative());
	return NativeRef->Native->getLinearVelocityDamping();
}

bool FWireBarrier::GetRenderListEmpty() const
{
	check(HasNative());
	return NativeRef->Native->getRenderListEmpty();
}

void FWireBarrier::AddRouteNode(FWireNodeBarrier& RoutingNode)
{
	check(HasNative());
	check(RoutingNode.HasNative());
	NativeRef->Native->add(RoutingNode.GetNative()->Native);
}

void FWireBarrier::AddWinch(FWireWinchBarrier& Winch)
{
	check(HasNative());
	check(Winch.HasNative());
	NativeRef->Native->add(Winch.GetNative()->Native);
}

bool FWireBarrier::IsInitialized() const
{
	check(HasNative());
	return NativeRef->Native->initialized();
}

double FWireBarrier::GetRestLength() const
{
	check(HasNative());
	const agx::Real LengthAGX = NativeRef->Native->getRestLength();
	const double Length = ConvertDistanceToUnreal<double>(LengthAGX);
	return Length;
}

FWireRenderIteratorBarrier FWireBarrier::GetRenderBeginIterator() const
{
	check(HasNative());
	return {std::make_unique<agxWire::RenderIterator>(NativeRef->Native->getRenderBeginIterator())};
}

FWireRenderIteratorBarrier FWireBarrier::GetRenderEndIterator() const
{
	check(HasNative());
	return {std::make_unique<agxWire::RenderIterator>(NativeRef->Native->getRenderEndIterator())};
}
