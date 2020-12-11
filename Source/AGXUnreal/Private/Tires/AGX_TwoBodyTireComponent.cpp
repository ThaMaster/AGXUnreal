#include "Tires/AGX_TwoBodyTireComponent.h"

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TwoBodyTireActor.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

UAGX_TwoBodyTireComponent::UAGX_TwoBodyTireComponent()
{
	TireRigidBody.FallbackOwningActor = GetOwner();
	HubRigidBody.FallbackOwningActor = GetOwner();
}

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

	// This reflects the behaviour of the AGX Dynamics agxModel::TwoBodyTire where a local transform
	// relative to the tire Rigid Body is used to define the final transform of the Tire model. The
	// axis of rotation is along the y-axis of this final transform in AGX Dynamics, which
	// corresponds to the negative y-axis in Unreal.
	FVector Pos = TireBody->GetComponentTransform().TransformPositionNoScale(LocalLocation);
	FQuat Rot = TireBody->GetComponentTransform().TransformRotation(LocalRotation.Quaternion());

	return FTransform(Rot, Pos);
}

void UAGX_TwoBodyTireComponent::CopyFrom(const FTwoBodyTireBarrier& Barrier)
{
	if (!Barrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s could not copy properties from Barrier because the Barrier does not have "
				 "a native object allocated."),
			*GetFName().ToString());
		return;
	}

	OuterRadius = Barrier.GetOuterRadius();
	InnerRadius = Barrier.GetInnerRadius();

	FTransform LocalTransform = Barrier.GetLocalTransform();
	LocalLocation = LocalTransform.GetLocation();
	LocalRotation = FRotator(LocalTransform.GetRotation());

	RadialStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::RADIAL);
	LateralStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::LATERAL);
	BendingStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::BENDING);
	TorsionalStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::TORSIONAL);

	RadialDamping = Barrier.GetDamping(FTwoBodyTireBarrier::RADIAL);
	LateralDamping = Barrier.GetDamping(FTwoBodyTireBarrier::LATERAL);
	BendingDamping = Barrier.GetDamping(FTwoBodyTireBarrier::BENDING);
	TorsionalDamping = Barrier.GetDamping(FTwoBodyTireBarrier::TORSIONAL);

	ImplicitFrictionMultiplier = Barrier.GetImplicitFrictionMultiplier();
}

bool UAGX_TwoBodyTireComponent::IsDefaultSubObjectOfTwoBodyTireActor() const
{
	AActor* Owner = GetOwner();
	return IsDefaultSubobject() && Owner != nullptr &&
		   Cast<AAGX_TwoBodyTireActor>(Owner) != nullptr;
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
	if (Barrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s UpdateNativeProperties, casting of FTireBarrier to FTwoBodyTireBarrier "
				 "failed."),
			*GetFName().ToString());
		return;
	}

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

void UAGX_TwoBodyTireComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (NativeBarrier->HasNative())
	{
		NativeBarrier->ReleaseNative();
	}
}

#if WITH_EDITOR
void UAGX_TwoBodyTireComponent::PostLoad()
{
	Super::PostLoad();

	// PostLoad is run when this component is created in a Blueprint or when a Blueprint containing
	// this component is instantiated in the level. This allows us to set the correct
	// FallbackOwningActor even for the Blueprint case where these FallbackOwningActor pointers will
	// point to the wrong object instances, even though they are set in the constructor. The reason
	// why this happens is still not known.
	TireRigidBody.FallbackOwningActor = GetOwner();
	HubRigidBody.FallbackOwningActor = GetOwner();
}
#endif

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
			TEXT(
				"Tire %s creation failed: at least one of the Rigid Bodies' Barriers was nullptr."),
			*GetFName().ToString());
		return Barrier;
	}

	Barrier->AllocateNative(
		TireBarrier, OuterRadius, HubBarrier, InnerRadius, LocalLocation,
		LocalRotation.Quaternion());

	return Barrier;
}
