// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "Templates/UnrealTypeTraits.h"

UAGX_ShovelComponent* FAGX_ShovelUtilities::GetShovelToModify(UAGX_ShovelComponent* Shovel)
{
	UAGX_ShovelComponent* Archetype = Cast<UAGX_ShovelComponent>(Shovel->GetArchetype());
	UAGX_ShovelComponent* CDO =
		Cast<UAGX_ShovelComponent>(UAGX_ShovelComponent::StaticClass()->GetDefaultObject(true));

	if (Archetype == CDO)
	{
		// This is a direct child of the class default object.
		// Do not edit the class default object.
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetShovelToModify: The archetype is the CDO, returning the original shovel %p."),
			Shovel);
		return Shovel;
	}

	UBlueprint* ShovelBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*Shovel);
	if (ShovelBlueprint)
	{
		AActor* Owner = Shovel->GetOwner();
		if (Owner != nullptr)
		{
			bool bPreviewInstance = FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
			UE_LOG(
				LogAGX, Warning, TEXT("GetShoveltoModify: bPreviewInstance=%d"),
				(int) bPreviewInstance);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning, TEXT("GetShovelToModify: Original shovel doesn't have an owner."));
		}

		// The Shovel is a template component in a Blueprint. We want to modify the Blueprint.
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetShovelToModify: The shovel has a blueprint, returning the original shovel %p"),
			Shovel);
		return Shovel;
	}

	UBlueprint* ArchetypeBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*Archetype);
	if (ArchetypeBlueprint)
	{
		AActor* Owner = Archetype->GetOwner();
		if (Owner != nullptr)
		{
			bool bPreviewInstance = FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
			UE_LOG(
				LogAGX, Warning, TEXT("GetShoveltoModify: bPreviewInstance=%d"),
				(int) bPreviewInstance);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("GetShovelToModify: Archetype shovel doesn't have an owner."));
		}
		// The archetype is a template component in a Blueprint. We want to modify the Blueprint.
		// Do we really?
		// Can we tell the difference between a behind-the-scenes Blueprint Editor shovel and an
		// in level instance on the Blueprint?
		// We want to return the Archetype only if the given shovel is a behind-the-scene instance.
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetShovelToModify: Archetype has a blueprint, returning the archetype %p"),
			Archetype);
		return Archetype;
	}

	// Anything else to test?

	UE_LOG(
		LogAGX, Warning, TEXT("GetShovelToModify: Base case, returning the original shovel%p"),
		Shovel);
	return Shovel;
}

void FAGX_ShovelUtilities::TruncateForDetailsPanel(double& Value)
{
	// See comment in header file.
	// At the time of writing the format specifier exposed for double in UnrealTypeTraits.h is %f.
	// Value = FCString::Atod(*FString::Printf(TEXT("%f"), Value));
	Value = FCString::Atod(*FString::Printf(TFormatSpecifier<double>::GetFormatSpecifier(), Value));
}


void FAGX_ShovelUtilities::TruncateForDetailsPanel(FVector& Values)
{
	TruncateForDetailsPanel(Values.X);
	TruncateForDetailsPanel(Values.Y);
	TruncateForDetailsPanel(Values.Z);
}
