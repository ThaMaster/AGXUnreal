// Copyright 2022, Algoryx Simulation AB.


#include "Terrain/ShovelBarrier.h"

// AGXUnrealBarrier includes.
#include "AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "agxTerrain/Shovel.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "Math/TwoVectors.h"
#include "Math/Vector.h"

FShovelBarrier::FShovelBarrier()
	: NativeRef {new FShovelRef}
{
}

FShovelBarrier::FShovelBarrier(std::unique_ptr<FShovelRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FShovelBarrier::FShovelBarrier(FShovelBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FShovelBarrier::~FShovelBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the
	// definition, not just the forward declaration, of FShovelRef.
}

void FShovelBarrier::SetTopEdge(const FTwoVectors& TopEdgeUnreal)
{
	check(HasNative());
	agx::Line TopEdgeAGX = ConvertDisplacement(TopEdgeUnreal);
	NativeRef->Native->setTopEdge(TopEdgeAGX);
}

void FShovelBarrier::SetCuttingEdge(const FTwoVectors& CuttingEdge)
{
	check(HasNative());
	agx::Line CuttingEdgeAGX = ConvertDisplacement(CuttingEdge);
	NativeRef->Native->setCuttingEdge(CuttingEdgeAGX);
}

void FShovelBarrier::SetVerticalBladeSoilMergeDistance(double VerticalBladeSoilMergeDistance)
{
	check(HasNative());
	NativeRef->Native->setVerticalBladeSoilMergeDistance(
		ConvertDistanceToAgx(VerticalBladeSoilMergeDistance));
}

double FShovelBarrier::GetVerticalBladeSoilMergeDistance() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getVerticalBladeSoilMergeDistance());
}

void FShovelBarrier::SetNoMergeExtensionDistance(double NoMergeExtensionDistance)
{
	check(HasNative());
	NativeRef->Native->setNoMergeExtensionDistance(ConvertDistanceToAgx(NoMergeExtensionDistance));
}

double FShovelBarrier::GetNoMergeExtensionDistance() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getNoMergeExtensionDistance());
}

void FShovelBarrier::SetPenetrationForceScaling(double PenetrationForceScaling)
{
	check(HasNative());
	NativeRef->Native->setPenetrationForceScaling(PenetrationForceScaling);
}

double FShovelBarrier::GetPenetrationForceScaling() const
{
	check(HasNative());
	return NativeRef->Native->getPenetrationForceScaling();
}

void FShovelBarrier::SetAlwaysRemoveShovelContacts(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setAlwaysRemoveShovelContacts(Enable);
}

bool FShovelBarrier::GetAlwaysRemoveShovelContacts() const
{
	check(HasNative());
	return NativeRef->Native->getAlwaysRemoveShovelContacts();
}

bool FShovelBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FShovelBarrier::AllocateNative(
	FRigidBodyBarrier& Body, const FTwoVectors& TopEdge, const FTwoVectors& CuttingEdge,
	const FVector& CuttingDirection)
{
	check(!HasNative());
	agx::RigidBody* BodyAGX = Body.GetNative()->Native;
	const agx::Line TopEdgeAGX = ConvertDisplacement(TopEdge);
	const agx::Line CuttingEdgeAGX = ConvertDisplacement(CuttingEdge);
	const agx::Vec3 CuttingDirectionAGX = ConvertVector(CuttingDirection);
	NativeRef->Native =
		new agxTerrain::Shovel(BodyAGX, TopEdgeAGX, CuttingEdgeAGX, CuttingDirectionAGX);
}

FShovelRef* FShovelBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FShovelRef* FShovelBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FShovelBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
