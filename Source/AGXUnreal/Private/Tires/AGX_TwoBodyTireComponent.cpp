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

FTransform UAGX_TwoBodyTireComponent::GetGlobalTireTransform() const
{
	UAGX_RigidBodyComponent* TireBody = GetTireRigidBody();
	if (TireBody == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s GetGlobalTireTransform failed, Tire Rigid Body was nullptr."),
			*GetFName().ToString());
		return FTransform::Identity;
	}

	// This reflects the behaviour of the agxModel::TwoBodyTire where a local transform relative
	// to the tire Rigid Body is used to define the final transform of the Tire model. The axis of
	// rotation is along the y-axis of this final transform in AGX Dynamics, which corresponds to
	// the negative y-axis in Unreal.
	FVector Pos = TireBody->GetComponentTransform().TransformPositionNoScale(LocalLocation);
	FQuat Rot = TireBody->GetComponentTransform().TransformRotation(LocalRotation.Quaternion());

	return FTransform(Rot, Pos);
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

	Barrier->AllocateNative(
		TireBarrier, OuterRadius, HubBarrier, InnerRadius, LocalLocation, LocalRotation.Quaternion());

	return Barrier;
}
