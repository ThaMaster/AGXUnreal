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
};
