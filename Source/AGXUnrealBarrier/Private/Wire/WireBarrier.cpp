// Copyright 2023, Algoryx Simulation AB.

#include "Wire/WireBarrier.h"

// AGX Unreal includes.
#include "AGXBarrierFactories.h"
#include "AGXRefs.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "NativeBarrier.impl.h"
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

// Linux and Windows treat symbol visibility differently and require the _API macro to be in
// different places for explicit template specializations.
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
	agx::Real RadiusAGX = ConvertDistanceToAGX(Radius);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInvToAGX(ResolutionPerUnitLength);
	NativeRef->Native = new agxWire::Wire(RadiusAGX, ResolutionPerUnitLengthAGX);
	PostNativeChanged();
}

void FWireBarrier::SetRadius(float Radius)
{
	check(HasNative());
	const agx::Real RadiusAGX = ConvertDistanceToAGX(Radius);
	NativeRef->Native->setRadius(RadiusAGX);
}

float FWireBarrier::GetRadius() const
{
	check(HasNative());
	const agx::Real RadiusAGX = NativeRef->Native->getRadius();
	const float Radius = ConvertDistanceToUnreal<float>(RadiusAGX);
	return Radius;
}

void FWireBarrier::SetResolutionPerUnitLength(float InResolution)
{
	check(HasNative());
	const agx::Real ResolutionAGX = ConvertDistanceInvToAGX(InResolution);
	NativeRef->Native->setResolutionPerUnitLength(ResolutionAGX);
}

float FWireBarrier::GetResolutionPerUnitLength() const
{
	check(HasNative());
	const agx::Real ResolutionAGX = NativeRef->Native->getResolutionPerUnitLength();
	const float Resolution = ConvertDistanceInvToUnreal<float>(ResolutionAGX);
	return Resolution;
}

void FWireBarrier::SetEnableCollisions(bool CanCollide)
{
	check(HasNative());
	NativeRef->Native->setEnableCollisions(CanCollide);
}

bool FWireBarrier::GetEnableCollisions() const
{
	check(HasNative());
	return NativeRef->Native->getEnableCollisions();
}

void FWireBarrier::AddCollisionGroup(const FName& GroupName)
{
	check(HasNative());
	AGX_CHECK(NativeRef->Native->getGeometryController() != nullptr);

	// Add collision group as (hashed) unsigned int.
	NativeRef->Native->getGeometryController()->addGroup(
		StringTo32BitFnvHash(GroupName.ToString()));
}

void FWireBarrier::AddCollisionGroups(const TArray<FName>& GroupNames)
{
	for (auto& GroupName : GroupNames)
	{
		AddCollisionGroup(GroupName);
	}
}

void FWireBarrier::RemoveCollisionGroup(const FName& GroupName)
{
	check(HasNative());
	AGX_CHECK(NativeRef->Native->getGeometryController() != nullptr);

	// Remove collision group as (hashed) unsigned int.
	NativeRef->Native->getGeometryController()->removeGroup(
		StringTo32BitFnvHash(GroupName.ToString()));
}

TArray<FName> FWireBarrier::GetCollisionGroups() const
{
	check(HasNative());

	agxWire::WireGeometryController* Gc = NativeRef->Native->getGeometryController();
	AGX_CHECK(Gc != nullptr);

	TArray<FName> Result;
	for (const agx::Name& Name : Gc->getGroupNames())
	{
		Result.Add(FName(*Convert(Name)));
	}

	for (const agx::UInt32 Id : Gc->getGroupIds())
	{
		Result.Add(FName(*FString::FromInt(Id)));
	}

	return Result;
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

void FWireBarrier::ClearMaterial()
{
	check(HasNative());
	NativeRef->Native->setMaterial(nullptr);
}

FShapeMaterialBarrier FWireBarrier::GetMaterial() const
{
	check(HasNative());
	agx::Material* Material = NativeRef->Native->getMaterial();
	return AGXBarrierFactories::CreateShapeMaterialBarrier(Material);
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

FWireNodeBarrier FWireBarrier::GetFirstNode() const
{
	check(HasNative());
	return AGXBarrierFactories::CreateWireNodeBarrier(NativeRef->Native->getFirstNode());
}

FWireNodeBarrier FWireBarrier::GetLastNode() const
{
	check(HasNative());
	return AGXBarrierFactories::CreateWireNodeBarrier(NativeRef->Native->getLastNode());
}

FWireWinchBarrier FWireBarrier::GetBeginWinch() const
{
	check(HasNative());
	agxWire::WireWinchController* Winch = NativeRef->Native->getWinchController(0);
	return AGXBarrierFactories::CreateWireWinchBarrier(Winch);
}

FWireWinchBarrier FWireBarrier::GetEndWinch() const
{
	check(HasNative());
	agxWire::WireWinchController* Winch = NativeRef->Native->getWinchController(1);
	return AGXBarrierFactories::CreateWireWinchBarrier(Winch);
}

FWireWinchBarrier FWireBarrier::GetWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
		case EWireSide::None:
			return FWireWinchBarrier();
	}
	UE_LOG(LogAGX, Error, TEXT("Invalid wire side passed to FWireBarrier::GetWinch."));
	return FWireWinchBarrier();
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

double FWireBarrier::GetMass() const
{
	check(HasNative());
	return NativeRef->Native->getMass();
}

double FWireBarrier::GetTension() const
{
	check(HasNative());
	agxWire::WireSegmentTensionData Data = NativeRef->Native->getTension(agx::Real(0.0));

	/// \todo Do tension need conversion to Unreal units?
	return Data.raw;
}

bool FWireBarrier::Attach(FWireWinchBarrier& Winch, bool bBegin)
{
	check(HasNative());
	check(Winch.HasNative());
	return NativeRef->Native->attach(Winch.GetNative()->Native, bBegin);
}

bool FWireBarrier::Detach(bool bBegin)
{
	check(HasNative());
	return NativeRef->Native->detach(bBegin);
}

bool FWireBarrier::Detach(FWireWinchBarrier& Winch)
{
	check(HasNative());
	if (!Winch.HasNative())
	{
		return false;
	}
	agxWire::Wire* NativeWire = NativeRef->Native;
	agxWire::WireWinchController* NativeWinch = Winch.GetNative()->Native;
	bool Detached = false;
	if (NativeWire->getWinchController(0) == NativeWinch)
	{
		Detached &= NativeWire->detach(true);
	}
	if (NativeWire->getWinchController(1) == NativeWinch)
	{
		Detached &= NativeWire->detach(false);
	}
	return Detached;
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

bool FWireBarrier::IsLumpedNode(const FWireNodeBarrier& Node) const
{
	check(HasNative());
	if (!Node.HasNative())
	{
		return false;
	}
	const agx::RigidBody* Body = Node.GetNative()->Native->getRigidBody();
	return NativeRef->Native->isLumpedNode(Body);
}

bool FWireBarrier::IsLumpedNode(const FWireRenderIteratorBarrier& It) const
{
	return IsLumpedNode(It.Get());
}

FString FWireBarrier::GetName() const
{
	check(HasNative());
	const agx::Name& NameAGX = NativeRef->Native->getName();
	const FString Name = Convert(NameAGX);
	return Name;
}

FGuid FWireBarrier::GetGuid() const
{
	check(HasNative());
	const agx::Uuid UuidAGX = NativeRef->Native->getUuid();
	FGuid Guid = Convert(UuidAGX);
	return Guid;
}
