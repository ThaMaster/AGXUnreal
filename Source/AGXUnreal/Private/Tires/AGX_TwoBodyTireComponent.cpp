#include "Tires/AGX_TwoBodyTireComponent.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"
#include "Tires/TwoBodyTireBarrier.h"

UAGX_RigidBodyComponent* UAGX_TwoBodyTireComponent::GetHubRigidBody() const
{
	return HubRigidBody.GetRigidBody();
}

UAGX_RigidBodyComponent* UAGX_TwoBodyTireComponent::GetTireRigidBody() const
{
	return TireRigidBody.GetRigidBody();
}

void UAGX_TwoBodyTireComponent::AllocateNative()
{
	NativeBarrier.Reset(CreateTwoBodyTireBarrier());
}

void UAGX_TwoBodyTireComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s UpdateNativeProperties failed, HasNative() returned false."),
			*GetFName().ToString());
		return;
	}

	FTwoBodyTireBarrier* Barrier = static_cast<FTwoBodyTireBarrier*>(GetNative());

	Barrier->SetStiffness(RadialStiffness, FTwoBodyTireBarrier::RADIAL);
	Barrier->SetStiffness(LateralStiffness, FTwoBodyTireBarrier::LATERAL);
	Barrier->SetStiffness(BendingStiffness, FTwoBodyTireBarrier::BENDING);
	Barrier->SetStiffness(TorsionalStiffness, FTwoBodyTireBarrier::TORSIONAL);

	Barrier->SetDamping(RadialDamping, FTwoBodyTireBarrier::RADIAL);
	Barrier->SetDamping(LateralDamping, FTwoBodyTireBarrier::LATERAL);
	Barrier->SetDamping(BendingDamping, FTwoBodyTireBarrier::BENDING);
	Barrier->SetDamping(TorsionalDamping, FTwoBodyTireBarrier::TORSIONAL);

	Barrier->SetImplicitFrictionMultiplier(ImplicitFrictionMultiplier);
}

FTwoBodyTireBarrier* UAGX_TwoBodyTireComponent::CreateTwoBodyTireBarrier()
{
	FTwoBodyTireBarrier* Barrier = new FTwoBodyTireBarrier;

	UAGX_RigidBodyComponent* TireBody = GetTireRigidBody();
	UAGX_RigidBodyComponent* HubBody = GetHubRigidBody();

	if (TireBody == nullptr || HubBody == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s creation failed: at least one of the Rigid Bodies used when trying to "
				 "create the Tire was nullptr."),
			*GetFName().ToString());
		return Barrier;
	}

	FRigidBodyBarrier* TireBarrier = TireBody->GetOrCreateNative();
	FRigidBodyBarrier* HubBarrier = HubBody->GetOrCreateNative();

	if (TireBarrier == nullptr || HubBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s creation failed: at least one of the Rigid Bodie's Natives was nullptr."),
			*GetFName().ToString());
		return Barrier;
	}

	Barrier->AllocateNative(TireBarrier, OuterRadius, HubBarrier, InnerRadius);

	return Barrier;
}
