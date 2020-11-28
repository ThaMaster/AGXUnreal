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
	const FRigidBodyBarrier* HubRigidBody, float InnerRadius)
{
	check(!HasNative());

	agx::RigidBody* TireBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(TireRigidBody);
	agx::RigidBody* HubBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(HubRigidBody);

	agxModel::TwoBodyTireRef Tire =
		new agxModel::TwoBodyTire(TireBody, OuterRadius, HubBody, InnerRadius);

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
