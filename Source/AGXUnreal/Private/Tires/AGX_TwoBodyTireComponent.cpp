// Copyright 2023, Algoryx Simulation AB.

#include "Tires/AGX_TwoBodyTireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Tires/AGX_TwoBodyTireActor.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

UAGX_TwoBodyTireComponent::UAGX_TwoBodyTireComponent()
{
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

void UAGX_TwoBodyTireComponent::CopyFrom(
	const FTwoBodyTireBarrier& Barrier, bool ForceOverwriteInstances)
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

	AGX_COPY_PROPERTY_FROM(ImportGuid, Barrier.GetGuid(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(OuterRadius, Barrier.GetOuterRadius(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(InnerRadius, Barrier.GetInnerRadius(), *this, ForceOverwriteInstances)

	const FTransform LocalTransform = Barrier.GetLocalTransform();
	AGX_COPY_PROPERTY_FROM(
		LocalLocation, LocalTransform.GetLocation(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		LocalRotation, FRotator(LocalTransform.GetRotation()), *this, ForceOverwriteInstances)

	AGX_COPY_PROPERTY_FROM(
		RadialStiffness, Barrier.GetStiffness(FTwoBodyTireBarrier::RADIAL), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		LateralStiffness, Barrier.GetStiffness(FTwoBodyTireBarrier::LATERAL), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		BendingStiffness, Barrier.GetStiffness(FTwoBodyTireBarrier::BENDING), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		TorsionalStiffness, Barrier.GetStiffness(FTwoBodyTireBarrier::TORSIONAL), *this,
		ForceOverwriteInstances)

	AGX_COPY_PROPERTY_FROM(
		RadialDamping, Barrier.GetDamping(FTwoBodyTireBarrier::RADIAL), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		LateralDamping, Barrier.GetDamping(FTwoBodyTireBarrier::LATERAL), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		BendingDamping, Barrier.GetDamping(FTwoBodyTireBarrier::BENDING), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		TorsionalDamping, Barrier.GetDamping(FTwoBodyTireBarrier::TORSIONAL), *this,
		ForceOverwriteInstances)

	AGX_COPY_PROPERTY_FROM(
		ImplicitFrictionMultiplier, Barrier.GetImplicitFrictionMultiplier(), *this,
		ForceOverwriteInstances)
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

void UAGX_TwoBodyTireComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAGX_TwoBodyTireComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after InitProperties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	//
	// The intention is to provide by default a local scope that is the Actor outer that this
	// component is part of. If the OwningActor is set anywhere else, such as in the Details Panel,
	// then that "else" should overwrite this value shortly.
	//
	// We use GetTypedOuter because we worry that in some cases the Owner may not yet have been set
	// but there will always be an outer chain. This worry may be unfounded.
	TireRigidBody.OwningActor = GetTypedOuter<AActor>();
	HubRigidBody.OwningActor = GetTypedOuter<AActor>();
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
