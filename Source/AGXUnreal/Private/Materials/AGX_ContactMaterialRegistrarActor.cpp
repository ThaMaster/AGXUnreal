#include "Materials/AGX_ContactMaterialRegistrarActor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "Materials/AGX_ContactMaterialInstance.h"

// Unreal Engine includes.
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"


#define LOCTEXT_NAMESPACE "AAGX_MaterialManager"

AAGX_ContactMaterialRegistrarActor::AAGX_ContactMaterialRegistrarActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAGX_ContactMaterialRegistrarActor::BeginPlay()
{
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
				TEXT("Contact material %s has at least one material that has not been set."),
				*ContactMaterial->GetName());

			continue;
		}

		// This will create the UAGX_ContactMaterialInstance if it did not already exist, initialize
		// its AGX native, and add it to the simulation.

		// It will also replace the passed in UAGX_ContactMaterialBase pointer with the new
		// instance, and return it.

		UAGX_ContactMaterialInstance* Instance =
			UAGX_ContactMaterialBase::GetOrCreateInstance(GetWorld(), ContactMaterial);
	}
}

void AAGX_ContactMaterialRegistrarActor::EndPlay(const EEndPlayReason::Type Reason)
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

void AAGX_ContactMaterialRegistrarActor::RemoveContactMaterial(
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
