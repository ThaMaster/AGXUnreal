#include "Wire/WireBarrier.h"

// AGX Unreal includes.
#include "Materials/ShapeMaterialBarrier.h"
#include "NativeBarrier.impl.h"
#include "AGXRefs.h"
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

#if PLATFORM_LINUX
template class FNativeBarrier<FWireRef>;
#elif PLATFORM_WINDOWS
template class AGXUNREALBARRIER_API FNativeBarrier<FWireRef>;
#endif

FWireBarrier::FWireBarrier()
	: Super()
{
}

FWireBarrier::FWireBarrier(std::unique_ptr<FWireRef>&& Native)
	: Super(std::move(Native))
{
}

FWireBarrier::FWireBarrier(FWireBarrier&& Other)
	: Super(std::move(Other))
{
}

FWireBarrier::~FWireBarrier()
{
}

/** Damping and Young's modulus for demonstration/experimentation purposes. Will be replaced
 * with Wire Material shortly. */
void FWireBarrier::AllocateNative(float Radius, float ResolutionPerUnitLength)
{
	check(!HasNative());
	PreNativeChanged();
	agx::Real RadiusAGX = ConvertDistance(Radius);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInv(ResolutionPerUnitLength);
	NativeRef->Native = new agxWire::Wire(RadiusAGX, ResolutionPerUnitLengthAGX);
	PostNativeChanged();
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

void FWireBarrier::SetMaterial(const FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	NativeRef->Native->setMaterial(Material.GetNative()->Native);
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
	const agx::Real LengthAGX = NativeRef->Native->getRestLength(false);
	const double Length = ConvertDistanceToUnreal<double>(LengthAGX);
	return Length;
}

double FWireBarrier::GetTension() const
{
	check(HasNative());
	agxWire::WireSegmentTensionData Data = NativeRef->Native->getTension(agx::Real(0.0));

	/// \todo Do tension need conversion to Unreal units?
	return Data.raw;
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
