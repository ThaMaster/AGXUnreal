// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSurfaceMaterialComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
#include "Sensors/AGX_LidarSurfaceMaterial.h"
#include "Sensors/AGX_SurfaceMaterialAssetUserData.h"
#include "Utilities/AGX_StringUtilities.h"

UAGX_LidarSurfaceMaterialComponent::UAGX_LidarSurfaceMaterialComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace AGX_LidarSurfaceMaterialComponent_helpers
{
	USceneComponent* FindFirstValidParent(const USceneComponent& Component)
	{
		TArray<USceneComponent*> Ancestors;
		Component.GetParentComponents(Ancestors);

		for (USceneComponent* Ancestor : Ancestors)
		{
			if (Ancestor->IsA<UStaticMeshComponent>() || Ancestor->IsA<UAGX_SimpleMeshComponent>())
				return Ancestor;
		}

		return nullptr;
	}

	TArray<USceneComponent*> FindValidSiblings(const USceneComponent& Component)
	{
		USceneComponent* Parent = Component.GetAttachParent();
		const TArray<USceneComponent*> Siblings = Parent->GetAttachChildren();

		TArray<USceneComponent*> ValidSiblings;
		for (USceneComponent* Sibling : Siblings)
		{
			if (Sibling->IsA<UStaticMeshComponent>() || Sibling->IsA<UAGX_SimpleMeshComponent>())
				ValidSiblings.Add(Sibling);
		}

		return ValidSiblings;
	}

	TArray<USceneComponent*> FindValidChildren(const USceneComponent& Component)
	{
		TArray<USceneComponent*> Children;
		Component.GetChildrenComponents(true, Children);

		TArray<USceneComponent*> ValidChildren;
		for (USceneComponent* Child : Children)
		{
			if (Child->IsA<UStaticMeshComponent>() || Child->IsA<UAGX_SimpleMeshComponent>())
				ValidChildren.Add(Child);
		}

		return ValidChildren;
	}

	TArray<USceneComponent*> GetComponentsFromSelection(
		const USceneComponent& Component, EAGX_LidarSurfaceMaterialAssignmentSelection Selection)
	{
		switch (Selection)
		{
			case EAGX_LidarSurfaceMaterialAssignmentSelection::LSM_Parent:
			{
				TArray<USceneComponent*> Components;
				if (auto Parent = FindFirstValidParent(Component))
					Components.Add(Parent);

				return Components;
			}
			case EAGX_LidarSurfaceMaterialAssignmentSelection::LSM_Siblings:
				return FindValidSiblings(Component);
			case EAGX_LidarSurfaceMaterialAssignmentSelection::LSM_Children:
				return FindValidChildren(Component);
		}

		UE_LOG(
			LogAGX, Error,
			TEXT("Unknown EAGX_LidarSurfaceMaterialAssignmentSelection given to "
				 "GetComponentsFromSelection."));
		return TArray<USceneComponent*>();
	}

	UAGX_SurfaceMaterialAssetUserData* Create(UActorComponent& Component)
	{
		UAssetUserData* Data =
			Component.GetAssetUserDataOfClass(UAGX_SurfaceMaterialAssetUserData::StaticClass());
		if (Data != nullptr)
			return nullptr; // Creation failed, data already exists.

		return NewObject<UAGX_SurfaceMaterialAssetUserData>(
			&Component, NAME_None, RF_Transactional);
	}
}

void UAGX_LidarSurfaceMaterialComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateMaterial();
	AssignMaterial();
}

void UAGX_LidarSurfaceMaterialComponent::UpdateMaterial()
{
	if (LidarSurfaceMaterial == nullptr)
		return;

	UWorld* World = GetWorld();
	UAGX_LidarSurfaceMaterial* Instance = LidarSurfaceMaterial->GetOrCreateInstance(World);
	check(Instance);

	// Swap asset to instance as we are now in-game.
	if (LidarSurfaceMaterial != Instance)
	{
		LidarSurfaceMaterial = Instance;
	}
}

void UAGX_LidarSurfaceMaterialComponent::AssignMaterial()
{
	using namespace AGX_LidarSurfaceMaterialComponent_helpers;
	TArray<USceneComponent*> Components = GetComponentsFromSelection(*this, Selection);
	for (auto C : Components)
	{
		if (C == nullptr)
			continue;

		UAGX_SurfaceMaterialAssetUserData* Data = Create(*C);
		if (Data == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Lidar Surface Material Component '%s' in '%s' tried to create and add "
					 "UAGX_SurfaceMaterialAssetUserData to '%s' but it already has an "
					 "UAGX_SurfaceMaterialAssetUserData. Make sure that multiple Lidar Surface "
					 "Material Components are not used to target the same Component '%s'. Doing "
					 "nothing."),
				*GetName(), *GetLabelSafe(GetOwner()), *C->GetName(), *C->GetName());
			continue;
		}

		Data->LidarSurfaceMaterial = LidarSurfaceMaterial;
		C->AddAssetUserData(Data);
	}
}
