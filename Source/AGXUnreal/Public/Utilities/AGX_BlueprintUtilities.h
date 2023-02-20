// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"

#if WITH_EDITOR

/**
 * A collection of utility functions for working with Blueprints.
 */
class AGXUNREAL_API FAGX_BlueprintUtilities
{
public:
	/**
	 * Convenience structure for representing a SCS Node search result.
	 */
	struct FAGX_BlueprintNodeSearchResult
	{
		FAGX_BlueprintNodeSearchResult(const UBlueprint& InBlueprint, USCS_Node* InNode)
			: Blueprint(InBlueprint)
			, FoundNode(InNode)
		{
		}
		const UBlueprint& Blueprint;
		USCS_Node* FoundNode = nullptr;
	};

	/**
	 * If the Component is inside a Blueprint, this function returns the SCS Node that has this
	 * component as its Component Template. If "SearchParentBlueprints" is
	 * set, any Blueprint parents will be searched as well. Returns the found SCS Node (if any) and
	 * the Blueprint it was found in, or the last Blueprint searched if no matching SCS Node
	 * was found.
	 */
	static FAGX_BlueprintNodeSearchResult GetSCSNodeFromComponent(
		const UBlueprint& Blueprint, const UActorComponent* Component, bool SearchParentBlueprints);

	/**
	 * Searches the Blueprint for an SCS Node of the given name. If "SearchParentBlueprints" is
	 * set, any Blueprint parents will be searched as well. Returns the found SCS Node (if any) and
	 * the Blueprint it was found in, or the last Blueprint searched if no matching SCS Node
	 * was found.
	 */
	static FAGX_BlueprintNodeSearchResult GetSCSNodeFromName(
		const UBlueprint& Blueprint, const FString& Name, bool SearchParentBlueprints);

	/**
	 * Check if the node name exists in the Blueprint.
	 */
	static bool NameExists(UBlueprint& Blueprint, const FString& Name);

	/**
	 * Returns the transform of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FTransform is returned.
	 */
	static FTransform GetTemplateComponentWorldTransform(const USceneComponent* Component);

	/**
	 * Sets the world transform of a template object in a Blueprint.
	 */
	static bool SetTemplateComponentWorldTransform(
		USceneComponent* Component, const FTransform& Transform,
		bool UpdateArchetypeInstances = true, bool ForceOverwriteInstances = false);

	/**
	 * Sets the relative transform of a template object in a Blueprint.
	 */
	static void SetTemplateComponentRelativeTransform(
		USceneComponent& Component, const FTransform& Transform,
		bool UpdateArchetypeInstances = true, bool ForceOverwriteInstances = false);

	/**
	 * Returns the location of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FVector is returned.
	 */
	static FVector GetTemplateComponentWorldLocation(USceneComponent* Component);

	/**
	 * Returns the rotation of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FRotator is returned.
	 */
	static FRotator GetTemplateComponentWorldRotation(USceneComponent* Component);

	/**
	 * Returns a list of all template components in a blueprint.
	 */
	static TArray<UActorComponent*> GetTemplateComponents(UBlueprint* Bp);
	static TArray<UActorComponent*> GetTemplateComponents(UBlueprintGeneratedClass* Bp);

	/**
	 * Returns the default template component name given a regular name.
	 */
	static FString ToTemplateComponentName(const FString& RegularName);

	/**
	 * Returns the regular name of a template component, i.e. the template component suffix is
	 * removed if present.
	 */
	static FString GetRegularNameFromTemplateComponentName(FString Name);

	/**
	 * Returns the attach parent of a template component.
	 */
	static UActorComponent* GetTemplateComponentAttachParent(UActorComponent* Component);

	/**
	 * Walks up the Blueprint inheritance chain one step and returns the immediate parent if it
	 * exists.
	 */
	static UBlueprint* GetParent(const UBlueprint& Child);

	/**
	 * Walks up the Blueprint inheritance chain and returns the "root" or outermost parent if it
	 * exists. If Child is the outermost parent, the Child itself is returned.
	 */
	static UBlueprint* GetOutermostParent(UBlueprint* Child);

	/**
	 * Returns the Blueprint that the template Component resides in.
	 */
	static UBlueprint* GetBlueprintFrom(const UActorComponent& Component);

	/**
	 * Makes Node a child of NewParent. If PreserveWorldTransform is set to true and the Node's
	 * TemplateComponent is a USceneComponent, it's world transform will be preserved, i.e. it's
	 * relative transform may change.
	 */
	static void ReParentNode(
		UBlueprint& Blueprint, USCS_Node& Node, USCS_Node& NewParent,
		bool PreserveWorldTransform = true);

	/**
	 * Searches through the node tree and returns the first Template Component matching the given
	 * type.
	 */
	template <typename T>
	static T* GetFirstComponentOfType(UBlueprint* Blueprint, bool SkipSceneRoot = false);
};

template <typename T>
T* FAGX_BlueprintUtilities::GetFirstComponentOfType(UBlueprint* Blueprint, bool SkipSceneRoot)
{
	if (Blueprint == nullptr || Blueprint->SimpleConstructionScript == nullptr)
	{
		return nullptr;
	}

	for (auto Node : Blueprint->SimpleConstructionScript->GetAllNodes())
	{
		if (T* Component = Cast<T>(Node->ComponentTemplate))
		{
			if (SkipSceneRoot)
			{
				if (Node != Blueprint->SimpleConstructionScript->GetDefaultSceneRootNode())
				{
					return Component;
				}
			}
			else
			{
				return Component;
			}
		}
	}

	return nullptr;
}
#endif // WITH_EDITOR
