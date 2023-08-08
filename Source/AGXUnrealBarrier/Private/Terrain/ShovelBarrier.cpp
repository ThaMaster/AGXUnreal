// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/ShovelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGXRefs.h"
#include "agxTerrain/Shovel.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

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

void FShovelBarrier::SetNumberOfTeeth(int32 NumberOfTeeth)
{
	check(HasNative());
	NativeRef->Native->setNumberOfTeeth(NumberOfTeeth);
}

int32 FShovelBarrier::GetNumberOfTeeth() const
{
	check(HasNative());
	return NativeRef->Native->getNumberOfTeeth();
}

void FShovelBarrier::SetToothLength(double ToothLength)
{
	check(HasNative());
	NativeRef->Native->setToothLength(ConvertDistanceToAGX(ToothLength));
}

double FShovelBarrier::GetToothLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getToothLength());
}

void FShovelBarrier::SetMinimumToothRadius(double MinimumToothRadius)
{
	check(HasNative());
	NativeRef->Native->setToothMinimumRadius(ConvertDistanceToAGX(MinimumToothRadius));
}

double FShovelBarrier::GetMinimumToothRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getToothMinimumRadius());
}

void FShovelBarrier::SetMaximumToothRadius(double MaximumToothRadius)
{
	check(HasNative());
	NativeRef->Native->setToothMaximumRadius(ConvertDistanceToAGX(MaximumToothRadius));
}

double FShovelBarrier::GetMaximumToothRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getToothMaximumRadius());
}

void FShovelBarrier::SetNoMergeExtensionDistance(double NoMergeExtensionDistance)
{
	check(HasNative());
	NativeRef->Native->setNoMergeExtensionDistance(ConvertDistanceToAGX(NoMergeExtensionDistance));
}

double FShovelBarrier::GetNoMergeExtensionDistance() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getNoMergeExtensionDistance());
}

void FShovelBarrier::SetMinimumSubmergedContactLengthFraction(
	double MinimumSubmergedContactLengthFraction)
{
	check(HasNative());
	NativeRef->Native->setMinimumSubmergedContactLengthFraction(
		MinimumSubmergedContactLengthFraction);
}

double FShovelBarrier::GetMinimumSubmergedContactLengthFraction() const
{
	check(HasNative());
	return NativeRef->Native->getMinimumSubmergedContactLengthFraction();
}

void FShovelBarrier::SetVerticalBladeSoilMergeDistance(double VerticalBladeSoilMergeDistance)
{
	check(HasNative());
	NativeRef->Native->setVerticalBladeSoilMergeDistance(
		ConvertDistanceToAGX(VerticalBladeSoilMergeDistance));
}

double FShovelBarrier::GetVerticalBladeSoilMergeDistance() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getVerticalBladeSoilMergeDistance());
}

void FShovelBarrier::SetSecondarySeparationDeadloadLimit(double SecondarySeparationDeadloadLimit)
{
	check(HasNative());
	NativeRef->Native->setSecondarySeparationDeadloadLimit(SecondarySeparationDeadloadLimit);
}

double FShovelBarrier::GetSecondarySeparationDeadloadLimit() const
{
	check(HasNative());
	return NativeRef->Native->getSecondarySeparationDeadloadLimit();
}

void FShovelBarrier::SetPenetrationDepthThreshold(double PenetrationDepthThreshold)
{
	check(HasNative());
	NativeRef->Native->setPenetrationDepthThreshold(
		ConvertDistanceToAGX(PenetrationDepthThreshold));
}

double FShovelBarrier::GetPenetrationDepthThreshold() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getPenetrationDepthThreshold());
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

void FShovelBarrier::SetMaximumPenetrationForce(double MaximumPenetrationForce)
{
	check(HasNative());
	NativeRef->Native->setMaxPenetrationForce(MaximumPenetrationForce);
}

double FShovelBarrier::GetMaximumPenetrationForce() const
{
	check(HasNative());
	return NativeRef->Native->getMaxPenetrationForce();
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

void FShovelBarrier::SetExcavationSettingsEnabled(EAGX_ExcavationMode Mode, bool Enable)
{
	check(HasNative());
	NativeRef->Native->getExcavationSettings(Convert(Mode)).setEnable(Enable);
}

bool FShovelBarrier::GetExcavationSettingsEnabled(EAGX_ExcavationMode Mode) const
{
	check(HasNative());
	return NativeRef->Native->getExcavationSettings(Convert(Mode)).getEnable();
}

void FShovelBarrier::SetExcavationSettingsEnableCreateDynamicMass(
	EAGX_ExcavationMode Mode, bool Enable)
{
	check(HasNative());
	NativeRef->Native->getExcavationSettings(Convert(Mode)).setEnableCreateDynamicMass(Enable);
}

bool FShovelBarrier::GetExcavationSettingsEnableCreateDynamicMass(EAGX_ExcavationMode Mode) const
{
	check(HasNative());
	return NativeRef->Native->getExcavationSettings(Convert(Mode)).getEnableCreateDynamicMass();
}

void FShovelBarrier::SetExcavationSettingsEnableForceFeedback(EAGX_ExcavationMode Mode, bool Enable)
{
	check(HasNative());
	NativeRef->Native->getExcavationSettings(Convert(Mode)).setEnableForceFeedback(Enable);
}

bool FShovelBarrier::GetExcavationSettingsEnableForceFeedback(EAGX_ExcavationMode Mode) const
{
	check(HasNative());
	return NativeRef->Native->getExcavationSettings(Convert(Mode)).getEnableForceFeedback();
}

bool FShovelBarrier::HasNative() const
{
	AGX_CHECK(NativeRef.get() != nullptr); // TEXT("Found an FShovelBarrier that does not have a
										   // NativeRef. This should never happen"));
	ensureMsgf(
		NativeRef.get() != nullptr, TEXT("AGXUnreal: Found an FShovelBarrier that does not have a "
										 "NativeRef. This should never happen."));
	if (NativeRef.get() == nullptr)
	{
		// We somehow ended up in a bad state where there is a Barrier that does not have a
		// NativeRef. This should never happen and in local builds we will already have aborted
		// after the failed AGX_CHECK above. In user applications we instead try to fix the
		// situation so that the application can keep running and not crash. The user will hopefully
		// see the ensureMsgf message printed above and let us know about the problem.
		//
		// HasNative is a const member function in that it never alters the salient value, but
		// here we need to do some under-the-hood cleanup. We assume that a FShoveBarrier will never
		// be an actual const value.
		FShovelBarrier* MutableThis = const_cast<FShovelBarrier*>(this);
		MutableThis->NativeRef.reset(new FShovelRef());
	}

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
	agx::Vec3 CuttingDirectionAGX = ConvertVector(CuttingDirection);

	// This is a fix for the error printed from AGX Dynamics where the tolerance of the length of
	// the Cutting Direction is so small that floating point precision is not enough, thus
	// triggering the error message even if set to length 100cm in Unreal.
	CuttingDirectionAGX.normalize();

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

uint64 FShovelBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FShovelBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxTerrain::Shovel*>(Address);
}

void FShovelBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
