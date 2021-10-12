#include "Materials/AGX_ContactMaterialRegistrarComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "Materials/AGX_ContactMaterialInstance.h"

#define LOCTEXT_NAMESPACE "UAGX_ContactMaterialRegistrarComponent"

UAGX_ContactMaterialRegistrarComponent::UAGX_ContactMaterialRegistrarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_ContactMaterialRegistrarComponent::RemoveContactMaterial(
	UAGX_ContactMaterialBase* ContactMaterial)
{
	if (ContactMaterial == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (!World->IsGameWorld())
	{
		// We assume that the ContactMaterials TArray is filled only with Assets (not Instances).
		if (ContactMaterial->GetAsset() == nullptr)
		{
			return;
		}
		ContactMaterials.Remove(ContactMaterial->GetAsset());
	}
	else
	{
		// We assume that the ContactMaterials TArray is filled only with Instances (not Assets).
		UAGX_ContactMaterialInstance* Instance =
			ContactMaterial->GetInstance();
		if (Instance == nullptr)
		{
			return;
		}
		ContactMaterials.Remove(Instance);
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			Sim->Remove(*Instance);
		}

		if (UAGX_ContactMaterialAsset* Asset = ContactMaterial->GetAsset())
		{
			Asset->ClearInstancePtr();
		}
	}
}

void UAGX_ContactMaterialRegistrarComponent::AddContactMaterial(
	UAGX_ContactMaterialBase* ContactMaterial)
{
	if (ContactMaterial == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (!World->IsGameWorld())
	{
		// We assume that the ContactMaterials TArray is filled only with Assets (not Instances).
		if (ContactMaterial->GetAsset() == nullptr)
		{
			return;
		}

		ContactMaterials.Add(ContactMaterial->GetAsset());
	}
	else
	{
		// We assume that the ContactMaterials TArray is filled only with Instances (not Assets).

		// Note: calling GetOrCreateInstance adds the Instance to the Simulation.
		UAGX_ContactMaterialInstance* Instance =
			UAGX_ContactMaterialBase::GetOrCreateInstance(GetWorld(), ContactMaterial);
		if (Instance == nullptr)
		{
			return;
		}

		ContactMaterials.Add(Instance);
	}
}

void UAGX_ContactMaterialRegistrarComponent::BeginPlay()
{
	Super::BeginPlay();

	// Convert all contact material pointers to point to initialized contact material instances.
	for (UAGX_ContactMaterialBase*& ContactMaterial : ContactMaterials)
	{
		if (!ContactMaterial)
		{
			continue;
		}

		if (!ContactMaterial->Material1 || !ContactMaterial->Material2)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Contact Material '%s' has at least one material that has not been set."),
				*ContactMaterial->GetName());

			continue;
		}

		// This will create the UAGX_ContactMaterialInstance if it did not already exist, initialize
		// its AGX native, and add it to the simulation.

		// It will also replace the passed in UAGX_ContactMaterialBase pointer with the new
		// instance, and return it.

		UAGX_ContactMaterialInstance* Instance =
			UAGX_ContactMaterialBase::GetOrCreateInstance(GetWorld(), ContactMaterial);

		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not create a Contact Material Instance for Contact Material '%s'."),
				*ContactMaterial->GetName());
			return;
		}

		// The Contact Material assets in the ContactMaterials TArray are swapped to Instances
		// during play.
		ContactMaterial = Instance;
	}
}

void UAGX_ContactMaterialRegistrarComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	for (UAGX_ContactMaterialBase* ContactMaterial : ContactMaterials)
	{
		if (!ContactMaterial)
		{
			continue;
		}

		RemoveContactMaterial(ContactMaterial->GetInstance(), Reason);
	}
}

void UAGX_ContactMaterialRegistrarComponent::RemoveContactMaterial(
	UAGX_ContactMaterialInstance* Instance, EEndPlayReason::Type Reason)
{
	if (Instance == nullptr)
	{
		return;
	}

	if (Instance->HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit)
	{
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			Sim->Remove(*Instance);
		}
	}

	if (Instance->HasNative())
	{
		Instance->GetNative()->ReleaseNative();
	}
}

#undef LOCTEXT_NAMESPACE
