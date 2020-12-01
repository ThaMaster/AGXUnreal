#include "Tires/TwoBodyTireBarrier.h"

// AGXUnreal includes.
#include "AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "AGX_LogCategory.h"
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxModel/TwoBodyTire.h>
#include "EndAGXIncludes.h"

FTwoBodyTireBarrier::FTwoBodyTireBarrier()
	: FTireBarrier()
{
}

FTwoBodyTireBarrier::FTwoBodyTireBarrier(std::unique_ptr<FTireRef> Native)
	: FTireBarrier(std::move(Native))
{
}

FTwoBodyTireBarrier::~FTwoBodyTireBarrier()
{
}

void FTwoBodyTireBarrier::AllocateNative(
	const FRigidBodyBarrier* TireRigidBody, float OuterRadius,
	const FRigidBodyBarrier* HubRigidBody, float InnerRadius, const FVector& LocalLocation,
	const FQuat& LocalRotation)
{
	check(!HasNative());

	agx::RigidBody* TireBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(TireRigidBody);
	agx::RigidBody* HubBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(HubRigidBody);
	agx::AffineMatrix4x4 LocalTransform = ConvertMatrix(LocalLocation, LocalRotation);

	agx::Real OuterRadiusAgx = ConvertDistanceToAgx<agx::Real>(OuterRadius);
	agx::Real InnerRadiusAgx = ConvertDistanceToAgx<agx::Real>(InnerRadius);

	agxModel::TwoBodyTireRef Tire = new agxModel::TwoBodyTire(
		TireBody, OuterRadiusAgx, HubBody, InnerRadiusAgx, LocalTransform);

	// Use of invalid agxModel::TwoBodyTire may lead to sudden crash during runtime.
	if (Tire->isValid())
	{
		NativeRef->Native = Tire;
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Error during creation of agxModel::TwoBodyTire, isValid() returned false."));
	}
}

float FTwoBodyTireBarrier::GetOuterRadius() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetOuterRadius failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return 0.f;
	}

	float RadiusAgx = Tire->getOuterRadius();
	return ConvertDistanceToUnreal<float>(RadiusAgx);
}

float FTwoBodyTireBarrier::GetInnerRadius() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetInnerRadius failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return 0.f;
	}

	float RadiusAgx = Tire->getInnerRadius();
	return ConvertDistanceToUnreal<float>(RadiusAgx);
}

FTransform FTwoBodyTireBarrier::GetLocalTransform() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetDamping failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return FTransform::Identity;
	}

	const agx::Frame* FrameAgx = Tire->getReferenceFrame();
	return ConvertLocalFrame(FrameAgx);
}

void FTwoBodyTireBarrier::SetDamping(float Damping, DeformationMode Mode)
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::SetDamping failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return;
	}

	Tire->setDampingCoefficient(Convert(Damping), Convert(Mode));
}

float FTwoBodyTireBarrier::GetDamping(DeformationMode Mode) const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetDamping failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return 0.f;
	}

	return Tire->getDampingCoefficient(Convert(Mode));
}

void FTwoBodyTireBarrier::SetStiffness(float Stiffness, DeformationMode Mode)
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::SetStiffness failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return;
	}

	Tire->setStiffness(Convert(Stiffness), Convert(Mode));
}

float FTwoBodyTireBarrier::GetStiffness(DeformationMode Mode) const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetStiffness failed: Cast from agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return 0.f;
	}

	return Tire->getStiffness(Convert(Mode));
}

void FTwoBodyTireBarrier::SetImplicitFrictionMultiplier(const FVector2D& Multiplier)
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::SetImplicitFrictionMultiplier failed: Cast from "
				 "agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return;
	}

	Tire->setImplicitFrictionMultiplier(Convert(Multiplier));
}

FVector2D FTwoBodyTireBarrier::GetImplicitFrictionMultiplier() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = dynamic_cast<agxModel::TwoBodyTire*>(NativeRef->Native.get());
	if (Tire == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetImplicitFrictionMultiplier failed: Cast from "
				 "agxModel::Tire to "
				 "agxModel::TwoBodyTire failed."));
		return FVector2D::ZeroVector;
	}

	agx::Vec2 AgxMultiplier = Tire->getImplicitFrictionMultiplier();
	return Convert(AgxMultiplier);
}
