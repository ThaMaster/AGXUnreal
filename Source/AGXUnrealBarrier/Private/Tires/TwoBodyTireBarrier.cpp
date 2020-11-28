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

	agx::RigidBody* Tire = FAGX_AgxDynamicsObjectsAccess::GetFrom(TireRigidBody);
	agx::RigidBody* Hub = FAGX_AgxDynamicsObjectsAccess::GetFrom(HubRigidBody);

	NativeRef->Native = new agxModel::TwoBodyTire(Tire, OuterRadius, Hub, InnerRadius);
}
