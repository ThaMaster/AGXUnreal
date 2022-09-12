// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

void UAGX_ShapeContactMergeSplitThresholds::SetMaxImpactSpeed_BP(float InMaxImpactSpeed)
{
	SetMaxImpactSpeed(FAGX_Real(InMaxImpactSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxImpactSpeed(FAGX_Real InMaxImpactSpeed)
{
	AGX_ASSET_SETTER_IMPL(MaxImpactSpeed, InMaxImpactSpeed, SetMaxImpactSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxImpactSpeed_BP() const
{
	return static_cast<float>(GetMaxImpactSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholds::GetMaxImpactSpeed() const
{
	AGX_ASSET_GETTER_IMPL(MaxImpactSpeed, GetMaxImpactSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeNormalSpeed_BP(
	float InMaxRelativeNormalSpeed)
{
	SetMaxRelativeNormalSpeed(FAGX_Real(InMaxRelativeNormalSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeNormalSpeed(
	FAGX_Real InMaxRelativeNormalSpeed)
{
	AGX_ASSET_SETTER_IMPL(
		MaxRelativeNormalSpeed, InMaxRelativeNormalSpeed, SetMaxRelativeNormalSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeNormalSpeed_BP() const
{
	return static_cast<float>(GetMaxRelativeNormalSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeNormalSpeed() const
{
	AGX_ASSET_GETTER_IMPL(MaxRelativeNormalSpeed, GetMaxRelativeNormalSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeTangentSpeed_BP(
	float InMaxRelativeTangentSpeed)
{
	SetMaxRelativeTangentSpeed(FAGX_Real(InMaxRelativeTangentSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeTangentSpeed(
	FAGX_Real InMaxRelativeTangentSpeed)
{
	AGX_ASSET_SETTER_IMPL(
		MaxRelativeTangentSpeed, InMaxRelativeTangentSpeed, SetMaxRelativeTangentSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeTangentSpeed_BP() const
{
	return static_cast<float>(GetMaxRelativeTangentSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeTangentSpeed() const
{
	AGX_ASSET_GETTER_IMPL(MaxRelativeTangentSpeed, GetMaxRelativeTangentSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRollingSpeed_BP(float InMaxRollingSpeed)
{
	SetMaxRollingSpeed(FAGX_Real(InMaxRollingSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRollingSpeed(FAGX_Real InMaxRollingSpeed)
{
	AGX_ASSET_SETTER_IMPL(MaxRollingSpeed, InMaxRollingSpeed, SetMaxRollingSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxRollingSpeed_BP() const
{
	return static_cast<float>(GetMaxRollingSpeed());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholds::GetMaxRollingSpeed() const
{
	AGX_ASSET_GETTER_IMPL(MaxRollingSpeed, GetMaxRollingSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetNormalAdhesion_BP(float InNormalAdhesion)
{
	SetNormalAdhesion(FAGX_Real(InNormalAdhesion));
}

void UAGX_ShapeContactMergeSplitThresholds::SetNormalAdhesion(FAGX_Real InNormalAdhesion)
{
	AGX_ASSET_SETTER_IMPL(NormalAdhesion, InNormalAdhesion, SetNormalAdhesion);
}

float UAGX_ShapeContactMergeSplitThresholds::GetNormalAdhesion_BP() const
{
	return static_cast<float>(GetNormalAdhesion());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholds::GetNormalAdhesion() const
{
	AGX_ASSET_GETTER_IMPL(NormalAdhesion, GetNormalAdhesion);
}

void UAGX_ShapeContactMergeSplitThresholds::SetTangentialAdhesion_BP(float InTangentialAdhesion)
{
	SetTangentialAdhesion(FAGX_Real(InTangentialAdhesion));
}

void UAGX_ShapeContactMergeSplitThresholds::SetTangentialAdhesion(FAGX_Real InTangentialAdhesion)
{
	AGX_ASSET_SETTER_IMPL(TangentialAdhesion, InTangentialAdhesion, SetTangentialAdhesion);
}

float UAGX_ShapeContactMergeSplitThresholds::GetTangentialAdhesion_BP() const
{
	return static_cast<float>(GetTangentialAdhesion());
}

FAGX_Real UAGX_ShapeContactMergeSplitThresholds::GetTangentialAdhesion() const
{
	AGX_ASSET_GETTER_IMPL(TangentialAdhesion, GetTangentialAdhesion);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaySplitInGravityField(
	bool bInMaySplitInGravityField)
{
	AGX_ASSET_SETTER_IMPL(
		bMaySplitInGravityField, bInMaySplitInGravityField, SetMaySplitInGravityField);
}

bool UAGX_ShapeContactMergeSplitThresholds::GetMaySplitInGravityField() const
{
	AGX_ASSET_GETTER_IMPL(bMaySplitInGravityField, GetMaySplitInGravityField);
}

void UAGX_ShapeContactMergeSplitThresholds::SetSplitOnLogicalImpact(bool bInSplitOnLogicalImpact)
{
	AGX_ASSET_SETTER_IMPL(bSplitOnLogicalImpact, bInSplitOnLogicalImpact, SetSplitOnLogicalImpact);
}

bool UAGX_ShapeContactMergeSplitThresholds::GetSplitOnLogicalImpact() const
{
	AGX_ASSET_GETTER_IMPL(bSplitOnLogicalImpact, GetSplitOnLogicalImpact);
}

#if WITH_EDITOR
void UAGX_ShapeContactMergeSplitThresholds::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ShapeContactMergeSplitThresholds::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShapeContactMergeSplitThresholds::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxImpactSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxImpactSpeed, SetMaxImpactSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxRelativeNormalSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRelativeNormalSpeed, SetMaxRelativeNormalSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxRelativeTangentSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRelativeTangentSpeed, SetMaxRelativeTangentSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxRollingSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRollingSpeed, SetMaxRollingSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, NormalAdhesion),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(NormalAdhesion, SetNormalAdhesion) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, TangentialAdhesion),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TangentialAdhesion, SetTangentialAdhesion) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, bMaySplitInGravityField),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(bMaySplitInGravityField, SetMaySplitInGravityField) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, bSplitOnLogicalImpact),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(bSplitOnLogicalImpact, SetSplitOnLogicalImpact) });
}
#endif

UAGX_ShapeContactMergeSplitThresholds* UAGX_ShapeContactMergeSplitThresholds::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_ShapeContactMergeSplitThresholds* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeContactMergeSplitThresholds::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

void UAGX_ShapeContactMergeSplitThresholds::CreateNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_ShapeContactMergeSplitThresholds "
					 "'%s' who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	NativeBarrier.Reset(new FShapeContactMergeSplitThresholdsBarrier());
	NativeBarrier->AllocateNative();
	AGX_CHECK(HasNative());

	SetNativeProperties(*NativeBarrier);
}

FShapeContactMergeSplitThresholdsBarrier* UAGX_ShapeContactMergeSplitThresholds::GetOrCreateNative(
	UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_ShapeContactMergeSplitThresholds '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}

	return NativeBarrier.Get();
}

bool UAGX_ShapeContactMergeSplitThresholds::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_ShapeContactMergeSplitThresholds* UAGX_ShapeContactMergeSplitThresholds::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholds& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());
	AGX_CHECK(!Source.IsInstance());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ShapeContactMergeSplitThresholds>(
		Outer, UAGX_ShapeContactMergeSplitThresholds::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

bool UAGX_ShapeContactMergeSplitThresholds::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;

	// Internal testing the hypothesis that UObject::IsAsset is a valid inverse of this function.
	// @todo Consider removing this function and instead use UObject::IsAsset, if the below check
	// has never failed.
	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}

void UAGX_ShapeContactMergeSplitThresholds::CopyProperties(
	UAGX_ShapeContactMergeSplitThresholds& Source)
{
	// Todo: implement.
}

void UAGX_ShapeContactMergeSplitThresholds::SetNativeProperties(
	FShapeContactMergeSplitThresholdsBarrier& Barrier)
{
	// TODO: implement.
}
