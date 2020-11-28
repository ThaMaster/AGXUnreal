#include "Tires/TwoBodyTireBarrier.h"

// AGXUnreal includes.
#include "AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "AGX_LogCategory.h"
#include "AGX_AgxDynamicsObjectsAccess.h"

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
