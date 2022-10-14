// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
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
	* Check if the node name exists in the Blueprint.
	*/
	static bool NameExists(UBlueprint& Blueprint, const FString& Name);

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

	/*
	 * Walks up the Blueprint inheritance chain and returns the "root" or outermost parent if it
	 * exists. If Child is the outermost parent, the Child itself is returned.
	 */
	static UBlueprint* GetOutermostParent(UBlueprint* Child);

	/*
	* Retrurns the Blueprint that the template Component resides in.
	*/
	static UBlueprint* GetBlueprintFrom(const UActorComponent& Component);

	/*
	* Checks whether the component is a template Component.
	*/
	static bool IsTemplateComponent(const UActorComponent& Component);

	/*
	 * Searches through the node tree and returns the first Template Component matching the given
	 * type.
	 */
	template <typename T>
	static T* GetFirstComponentOfType(UBlueprint* Blueprint);
};

template <typename T>
T* FAGX_BlueprintUtilities::GetFirstComponentOfType(UBlueprint* Blueprint)
{
	if (Blueprint == nullptr || Blueprint->SimpleConstructionScript == nullptr)
	{
		return nullptr;
	}

	for (auto Node : Blueprint->SimpleConstructionScript->GetAllNodes())
	{
		if (T* Component = Cast<T>(Node->ComponentTemplate))
		{
			return Component;
		}
	}

	return nullptr;
}