// Copyright 2023, Algoryx Simulation AB.

#include "Utilities/AGX_BlueprintUtilities.h"

// AGX Dynamics for Unreal includes.
#include "Components/ActorComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"

namespace AGX_BlueprintUtilities_helpers
{
	UBlueprintGeneratedClass* GetBlueprintGeneratedClass(UActorComponent* Component)
	{
		if (Component == nullptr)
		{
			return nullptr;
		}
		return Cast<UBlueprintGeneratedClass>(Component->GetOuter());
	}

	template <typename T>
	TArray<UActorComponent*> GetTemplateComponents(T* Bp)
	{
		TArray<UActorComponent*> Components;
		if (Bp == nullptr)
		{
			return Components;
		}

		for (USCS_Node* Node : Bp->SimpleConstructionScript->GetAllNodes())
		{
			if (UActorComponent* Component = Node->ComponentTemplate)
			{
				Components.Add(Component);
			}
		}

		return Components;
	}
}

USCS_Node* FAGX_BlueprintUtilities::GetSCSNodeFromComponent(UActorComponent* Component)
{
	using namespace AGX_BlueprintUtilities_helpers;
	if (Component == nullptr)
	{
		return nullptr;
	}

	UBlueprintGeneratedClass* Blueprint = GetBlueprintGeneratedClass(Component);
	if (Blueprint == nullptr)
	{
		return nullptr;
	}

	TArray<USCS_Node*> Nodes = Blueprint->SimpleConstructionScript->GetAllNodes();
	USCS_Node** ComponentNode = Nodes.FindByPredicate(
		[Component](USCS_Node* Node) { return Node->ComponentTemplate == Component; });
	if (ComponentNode == nullptr)
	{
		return nullptr;
	}

	return *ComponentNode;
}

FTransform FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(USceneComponent* Component)
{
	using namespace AGX_BlueprintUtilities_helpers;
	if (Component == nullptr)
	{
		return FTransform::Identity;
	}

	UBlueprintGeneratedClass* Blueprint = GetBlueprintGeneratedClass(Component);
	if (Blueprint == nullptr)
	{
		return FTransform::Identity;
	}

	USCS_Node* ComponentNode = GetSCSNodeFromComponent(Component);
	if (ComponentNode == nullptr)
	{
		return FTransform::Identity;
	}

	// Build a chain of USCS_Nodes starting from root and going down to the Component's
	// USCS_Node.
	TArray<USCS_Node*> RootToComponentChain;
	USCS_Node* CurrentNode = ComponentNode;
	RootToComponentChain.Insert(CurrentNode, 0);
	while (USCS_Node* Parent = Blueprint->SimpleConstructionScript->FindParentNode(CurrentNode))
	{
		RootToComponentChain.Insert(Parent, 0);
		CurrentNode = Parent;
	}

	FTransform WorldTransform = FTransform::Identity;
	for (USCS_Node* Node : RootToComponentChain)
	{
		if (Node == nullptr || Node->ComponentTemplate == nullptr)
		{
			continue;
		}
		if (USceneComponent* SceneComponent = Cast<USceneComponent>(Node->ComponentTemplate))
		{
			const FTransform RelativeTransform = SceneComponent->GetRelativeTransform();
			FTransform::Multiply(&WorldTransform, &RelativeTransform, &WorldTransform);
		}
	}

	return WorldTransform;
}

bool FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
	USceneComponent* Component, const FTransform& Transform, bool UpdateArchetypeInstances)
{
	using namespace AGX_BlueprintUtilities_helpers;
	if (Component == nullptr)
	{
		return false;
	}

	UBlueprintGeneratedClass* Blueprint = GetBlueprintGeneratedClass(Component);
	if (Blueprint == nullptr)
	{
		return false;
	}

	USCS_Node* ComponentNode = GetSCSNodeFromComponent(Component);
	if (ComponentNode == nullptr)
	{
		return false;
	}

	USCS_Node* ParentNode = Blueprint->SimpleConstructionScript->FindParentNode(ComponentNode);
	if (ParentNode == nullptr || ParentNode->ComponentTemplate == nullptr)
	{
		return false;
	}

	USceneComponent* ParentSceneComponent = Cast<USceneComponent>(ParentNode->ComponentTemplate);
	if (ParentSceneComponent == nullptr)
	{
		return false;
	}

	const FVector OrigRelLocation = Component->GetRelativeLocation();
	const FRotator OrigRelRotation = Component->GetRelativeRotation();

	const FTransform ParentWorldTransform =
		GetTemplateComponentWorldTransform(ParentSceneComponent);
	const FTransform NewRelTransform = Transform.GetRelativeTransform(ParentWorldTransform);
	Component->Modify();
	Component->SetRelativeTransform(NewRelTransform);

	if (!UpdateArchetypeInstances)
	{
		// We are done.
		return true;
	}

	// Update any archetype instances that are "in sync" with the template component.
	// Note: Using SetWorldTransform fails here because that function internally uses the
	// transform of the attach-parent to calculate a new relative transform and sets that. When
	// dealing with objects inside a Blueprint, the attach-parent is not set. Therefore we must
	// stick to using only RelativeLocation/Rotation.
	for (USceneComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
	{
		// Only write to the Archetype Instances if they are currently in sync with this
		// template.
		if (Instance->GetRelativeLocation() == OrigRelLocation &&
			Instance->GetRelativeRotation() == OrigRelRotation)
		{
			Instance->Modify();
			Instance->SetRelativeLocation(Component->GetRelativeLocation());
			Instance->SetRelativeRotation(Component->GetRelativeRotation());

			// The purpose of this function is to make sure the Instances get exactly the same
			// relative transform as the Archetype. However, SetRelativeLocation/Rotation does
			// some transformation calculations internally which in some cases result in (small)
			// rounding errors, which is enough to break the state in the Blueprint. We call the
			// Set..._Direct functions here to ensure that the RelativeLocation/Rotation matches
			// exactly with the archetype. The above calls are still needed because those make
			// sure the component is updated in the viewport without the need to recompile the
			// Blueprint.
			Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
			Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
		}
	}

	return true;
}

TArray<UActorComponent*> FAGX_BlueprintUtilities::GetTemplateComponents(
	UBlueprintGeneratedClass* Bp)
{
	return AGX_BlueprintUtilities_helpers::GetTemplateComponents(Bp);
}

TArray<UActorComponent*> FAGX_BlueprintUtilities::GetTemplateComponents(UBlueprint* Bp)
{
	return AGX_BlueprintUtilities_helpers::GetTemplateComponents(Bp);
}

FString FAGX_BlueprintUtilities::ToTemplateComponentName(const FString& RegularName)
{
	return RegularName + UActorComponent::ComponentTemplateNameSuffix;
}

FVector FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(USceneComponent* Component)
{
	return GetTemplateComponentWorldTransform(Component).GetLocation();
}

FRotator FAGX_BlueprintUtilities::GetTemplateComponentWorldRotation(USceneComponent* Component)
{
	return GetTemplateComponentWorldTransform(Component).Rotator();
}

UActorComponent* FAGX_BlueprintUtilities::GetTemplateComponentAttachParent(
	UActorComponent* Component)
{
	using namespace AGX_BlueprintUtilities_helpers;

	if (Component == nullptr)
		return nullptr;

	UBlueprintGeneratedClass* Blueprint = GetBlueprintGeneratedClass(Component);
	if (Blueprint == nullptr)
		return nullptr;

	USCS_Node* ParentNode =
		Blueprint->SimpleConstructionScript->FindParentNode(GetSCSNodeFromComponent(Component));
	if (ParentNode == nullptr)
		return nullptr;

	return ParentNode->ComponentTemplate;
}
