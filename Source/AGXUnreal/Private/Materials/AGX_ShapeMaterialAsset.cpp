// Copyright 2021, Algoryx Simulation AB.


#include "Materials/AGX_ShapeMaterialAsset.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_MaterialBase* UAGX_ShapeMaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_ShapeMaterialInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeMaterialInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR
void UAGX_ShapeMaterialAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// The name of the property that was changed.
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// The root property that contains the property that was changed.
	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL)
								   ? PropertyChangedEvent.MemberProperty->GetFName()
								   : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Surface))
	{
		WriteSurfacePropertyToInstance(PropertyName);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Bulk))
	{
		WriteBulkPropertyToInstance(PropertyName);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_MaterialBase, Wire))
	{
		WriteWirePropertyToInstance(PropertyName);
	}
}

void UAGX_ShapeMaterialAsset::WriteSurfacePropertyToInstance(const FName& PropertyName)
{
	if (Instance == nullptr)
	{
		return;
	}

	typedef FAGX_ShapeMaterialSurfaceProperties SurfaceProperties;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, bFrictionEnabled))
	{
		Instance->SetFrictionEnabled(Surface.bFrictionEnabled);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, Roughness))
	{
		Instance->SetRoughness(static_cast<float>(Surface.Roughness));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, Viscosity))
	{
		Instance->SetSurfaceViscosity(static_cast<float>(Surface.Viscosity));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, AdhesiveForce))
	{
		Instance->SetAdhesion(
			static_cast<float>(Surface.AdhesiveForce), static_cast<float>(Surface.AdhesiveOverlap));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(SurfaceProperties, AdhesiveOverlap))
	{
		Instance->SetAdhesion(
			static_cast<float>(Surface.AdhesiveForce), static_cast<float>(Surface.AdhesiveOverlap));
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Property: %s changed but not written to the ShapeMaterialInstance."),
			*PropertyName.ToString());
	}
}

void UAGX_ShapeMaterialAsset::WriteBulkPropertyToInstance(const FName& PropertyName)
{
	if (Instance == nullptr)
	{
		return;
	}

	typedef FAGX_ShapeMaterialBulkProperties BulkProperties;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, Density))
	{
		Instance->SetDensity(static_cast<float>(Bulk.Density));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, YoungsModulus))
	{
		Instance->SetYoungsModulus(static_cast<float>(Bulk.YoungsModulus));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, Viscosity))
	{
		Instance->SetBulkViscosity(static_cast<float>(Bulk.Viscosity));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, SpookDamping))
	{
		Instance->SetSpookDamping(static_cast<float>(Bulk.SpookDamping));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, MinElasticRestLength))
	{
		Instance->SetMinMaxElasticRestLength(
			static_cast<float>(Bulk.MinElasticRestLength),
			static_cast<float>(Bulk.MaxElasticRestLength));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(BulkProperties, MaxElasticRestLength))
	{
		Instance->SetMinMaxElasticRestLength(
			static_cast<float>(Bulk.MinElasticRestLength),
			static_cast<float>(Bulk.MaxElasticRestLength));
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Property: %s changed but not written to the ShapeMaterialInstance."),
			*PropertyName.ToString());
	}
}

void UAGX_ShapeMaterialAsset::WriteWirePropertyToInstance(const FName& PropertyName)
{
	if (Instance == nullptr)
	{
		return;
	}

	typedef FAGX_ShapeMaterialWireProperties WireProperties;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(WireProperties, YoungsModulusStretch))
	{
		Instance->SetYoungsModulusStretch(Wire.YoungsModulusStretch);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(WireProperties, YoungsModulusBend))
	{
		Instance->SetYoungsModulusBend(Wire.YoungsModulusBend);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(WireProperties, SpookDampingStretch))
	{
		Instance->SetSpookDampingStretch(Wire.SpookDampingStretch);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(WireProperties, SpookDampingBend))
	{
		Instance->SetSpookDampingBend(Wire.SpookDampingBend);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Property: %s changed but not written to the ShapeMaterialInstance."),
			*PropertyName.ToString());
	}
}
#endif
