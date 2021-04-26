#include "Wire/WireBarrier.h"

// AGX Unreal includes.
#include "TypeConversions.h"
#include "Wire/WireNodeBarrier.h"
#include "Wire/WireNodeRef.h"
#include "Wire/WireRef.h"

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

void FWireBarrier::AllocateNative(float Radius, float ResolutionPerUnitLength)
{
	check(!HasNative());
	agx::Real RadiusAGX = ConvertDistance(Radius);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInv(ResolutionPerUnitLength);
	NativeRef->Native = new agxWire::Wire(RadiusAGX, ResolutionPerUnitLengthAGX);
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

void FWireBarrier::AddRouteNode(FWireNodeBarrier& RoutingNode)
{
	check(HasNative());
	check(RoutingNode.HasNative());
	NativeRef->Native->add(RoutingNode.GetNative()->Native);
}

bool FWireBarrier::IsInitialized() const
{
	check(HasNative());
	return NativeRef->Native->initialized();
}

double FWireBarrier::GetRestLength() const
{
	check(HasNative());
	agx::Real LengthAGX = NativeRef->Native->getRestLength();
	double Length = ConvertDistanceToUnreal<double>(LengthAGX);
	return Length;
}
