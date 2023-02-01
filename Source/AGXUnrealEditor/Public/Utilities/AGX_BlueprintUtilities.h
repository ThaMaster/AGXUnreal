// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/SCS_Node.h"

/*
 * A collection of utility functions for working with Blueprints.
 */
class AGXUNREALEDITOR_API FAGX_BlueprintUtilities
{
public:
	/*
	 * If the Component is inside a Blueprint, this function returns the USCS_Node that has this
	 * component as its Component Template. Otherwise, this functions returns nullptr.
	 */
	static USCS_Node* GetSCSNodeFromComponent(UActorComponent* Component);

	/*
	 * Returns the transform of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FTransform is returned.
	 */
	static FTransform GetTemplateComponentWorldTransform(USceneComponent* Component);

	/*
	 * Sets the world transform of a template object in a Blueprint.
	 */
	static bool SetTemplateComponentWorldTransform(
		USceneComponent* Component, const FTransform& Transform,
		bool UpdateArchetypeInstances = true);

	/*
	 * Returns the location of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FVector is returned.
	 */
	static FVector GetTemplateComponentWorldLocation(USceneComponent* Component);

	/*
	 * Returns the rotation of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FRotator is returned.
	 */
	static FRotator GetTemplateComponentWorldRotation(USceneComponent* Component);

	/*
	 * Returns a list of all template components in a blueprint.
	 */
	static TArray<UActorComponent*> GetTemplateComponents(UBlueprint* Bp);
	static TArray<UActorComponent*> GetTemplateComponents(UBlueprintGeneratedClass* Bp);

	/*
	 * Returns the default template component name given a regular name.
	 */
	static FString ToTemplateComponentName(const FString& RegularName);

	/*
	 * Returns the attach parent of a template component.
	 */
	static UActorComponent* GetTemplateComponentAttachParent(UActorComponent* Component);
};
