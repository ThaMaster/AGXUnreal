// Author: VMC Motion Technologies Co., Ltd.


#include "Vehicle/TrackPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/TrackPropertiesRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include <Misc/AssertionMacros.h>


FTrackPropertiesBarrier::FTrackPropertiesBarrier()
	: NativeRef{ new FTrackPropertiesRef }
{
}

FTrackPropertiesBarrier::FTrackPropertiesBarrier(FTrackPropertiesBarrier&& Other)
	: NativeRef{ std::move(Other.NativeRef) }
{
}

FTrackPropertiesBarrier::FTrackPropertiesBarrier(std::unique_ptr<FTrackPropertiesRef> Native)
	: NativeRef(std::move(Native))
{
}

FTrackPropertiesBarrier::~FTrackPropertiesBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrackPropertiesRef.
}

bool FTrackPropertiesBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTrackPropertiesRef* FTrackPropertiesBarrier::GetNative()
{
	return NativeRef.get();
}

const FTrackPropertiesRef* FTrackPropertiesBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTrackPropertiesBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxVehicle::TrackProperties();
}

void FTrackPropertiesBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FGuid FTrackPropertiesBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

void FTrackPropertiesBarrier::SetHingeCompliance(double Compliance, int32 DOF)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, static_cast<agx::Hinge::DOF>(DOF));
}

void FTrackPropertiesBarrier::SetHingeComplianceTranslational(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeComplianceTranslational(Compliance);
}

void FTrackPropertiesBarrier::SetHingeComplianceRotational(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeComplianceRotational(Compliance);
}

double FTrackPropertiesBarrier::GetHingeCompliance(int32 DOF) const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(static_cast<agx::Hinge::DOF>(DOF));
}

void FTrackPropertiesBarrier::SetHingeDamping(double Damping, int32 DOF)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(Damping, static_cast<agx::Hinge::DOF>(DOF));
}

void FTrackPropertiesBarrier::SetHingeDampingTranslational(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDampingTranslational(Damping);
}

void FTrackPropertiesBarrier::SetHingeDampingRotational(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDampingRotational(Damping);
}

double FTrackPropertiesBarrier::GetHingeDamping(int32 DOF) const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(static_cast<agx::Hinge::DOF>(DOF));
}

void FTrackPropertiesBarrier::SetEnableHingeRange(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableHingeRange(bEnable);
}

bool FTrackPropertiesBarrier::GetEnableHingeRange() const
{
	check(HasNative());
	return NativeRef->Native->getEnableHingeRange();
}

void FTrackPropertiesBarrier::SetHingeRangeRange(FAGX_RealInterval MinMaxAngles)
{
	check(HasNative());
	agx::RangeReal RangeAGX = ConvertAngle(MinMaxAngles);
	NativeRef->Native->setHingeRangeRange(RangeAGX.lower(), RangeAGX.upper());
}

FAGX_RealInterval FTrackPropertiesBarrier::GetHingeRangeRange() const
{
	check(HasNative());
	const agx::RangeReal RangeAGX = NativeRef->Native->getHingeRangeRange();
	const FAGX_RealInterval RangeUnreal = ConvertAngle(RangeAGX);
	return RangeUnreal;
}

void FTrackPropertiesBarrier::SetEnableOnInitializeMergeNodesToWheels(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableOnInitializeMergeNodesToWheels(bEnable);
}

bool FTrackPropertiesBarrier::GetEnableOnInitializeMergeNodesToWheels() const
{
	check(HasNative());
	return NativeRef->Native->getEnableOnInitializeMergeNodesToWheels();
}

void FTrackPropertiesBarrier::SetEnableOnInitializeTransformNodesToWheels(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableOnInitializeTransformNodesToWheels(bEnable);
}

bool FTrackPropertiesBarrier::GetEnableOnInitializeTransformNodesToWheels() const
{
	check(HasNative());
	return NativeRef->Native->getEnableOnInitializeTransformNodesToWheels();
}

void FTrackPropertiesBarrier::SetTransformNodesToWheelsOverlap(double Overlap)
{
	check(HasNative());
	NativeRef->Native->setTransformNodesToWheelsOverlap(ConvertDistanceToAGX(Overlap));
}

double FTrackPropertiesBarrier::GetTransformNodesToWheelsOverlap() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getTransformNodesToWheelsOverlap());
}

void FTrackPropertiesBarrier::SetNodesToWheelsMergeThreshold(double MergeThreshold)
{
	check(HasNative());
	NativeRef->Native->setNodesToWheelsMergeThreshold(MergeThreshold);
}

double FTrackPropertiesBarrier::GetNodesToWheelsMergeThreshold() const
{
	check(HasNative());
	return NativeRef->Native->getNodesToWheelsMergeThreshold();
}

void FTrackPropertiesBarrier::SetNodesToWheelsSplitThreshold(double SplitThreshold)
{
	check(HasNative());
	NativeRef->Native->setNodesToWheelsSplitThreshold(SplitThreshold);
}

double FTrackPropertiesBarrier::GetNodesToWheelsSplitThreshold() const
{
	check(HasNative());
	return NativeRef->Native->getNodesToWheelsSplitThreshold();
}

void FTrackPropertiesBarrier::SetNumNodesIncludedInAverageDirection(uint32 NumIncludedNodes)
{
	check(HasNative());
	NativeRef->Native->setNumNodesIncludedInAverageDirection(NumIncludedNodes);
}

uint32 FTrackPropertiesBarrier::GetNumNodesIncludedInAverageDirection() const
{
	check(HasNative());
	return NativeRef->Native->getNumNodesIncludedInAverageDirection();
}

void FTrackPropertiesBarrier::SetMinStabilizingHingeNormalForce(double MinNormalForce)
{
	check(HasNative());
	NativeRef->Native->setMinStabilizingHingeNormalForce(MinNormalForce);
}

double FTrackPropertiesBarrier::GetMinStabilizingHingeNormalForce() const
{
	check(HasNative());
	return NativeRef->Native->getMinStabilizingHingeNormalForce();
}

void FTrackPropertiesBarrier::SetStabilizingHingeFrictionParameter(double FrictionParameter)
{
	check(HasNative());
	NativeRef->Native->setStabilizingHingeFrictionParameter(FrictionParameter);
}

double FTrackPropertiesBarrier::GetStabilizingHingeFrictionParameter() const
{
	check(HasNative());
	return NativeRef->Native->getStabilizingHingeFrictionParameter();
}
