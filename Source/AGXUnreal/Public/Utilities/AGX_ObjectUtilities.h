// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Containers/Array.h"
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Misc/EngineVersionComparison.h"

/**
 * A collection of generic helper functions that can be compiled for Runtime.
 *
 * \see AGX_EditorUtilities.h, which is for WITH_EDITOR builds.
 *
 * There are also other, more specialized, AGX_.+Utilities.h collections.
 */
class AGXUNREAL_API FAGX_ObjectUtilities
{
public:
	/**
	 * Returns array containing all child actors (and all their child actors)
	 * recursively, attached to the Parent actor passed as input argument.
	 * The parent itself is not included in the set.
	 */
	static void GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors);

	/*
	 * Checks whether the component is a template Component, i.e. it may have archetype instances.
	 */
	static bool IsTemplateComponent(const UActorComponent& Component);

	/**
	 * Give a list of pointer-to-base, return a new list with the elements that
	 * are of a particular derived type.
	 */
	template <typename UDerived, typename UBaseContainer>
	static TArray<UDerived*> Filter(const UBaseContainer& Collection);

	template <typename T>
	static T* FindFirstAncestorOfType(const USceneComponent& Start);

	/**
	 * Returns the number children of type T of the actor.
	 */
	template <typename T>
	static uint32 GetNumComponentsInActor(
		const AActor& Actor, bool bIncludeFromChildActors = false);

	template <typename T>
	static T* Get(const FComponentReference& Reference, const AActor* FallbackOwner);

	static const AActor* GetActor(
		const FComponentReference& Reference, const AActor* FallbackActor = nullptr)
	{
		const AActor* Actor =
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			Reference.OtherActor;
#else
			Reference.OtherActor.Get();
#endif
		return Actor != nullptr ? Actor : FallbackActor;
	}

	/*
	 * Returns any Archetype instances of the passed object. If the passed object is not an
	 * archetype, an empty TArray is returned.
	 * Looks recursively in the archetype tree and returns all Instances in it.
	 * Note that an Archetype Instance may itself be an Archetype.
	 */
	template <typename T>
	static TArray<T*> GetArchetypeInstances(T& Object);

#if WITH_EDITOR
	/**
	 * Saves (or re-saves) an asset to disk. The asset must have a valid Package setup before
	 * passing it to this function.
	 */
	static bool SaveAsset(UObject& Asset);
#endif

private:
	static void GetActorsTree(const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors);
};

template <typename T>
T* FAGX_ObjectUtilities::FindFirstAncestorOfType(const USceneComponent& Start)
{
	USceneComponent* Parent = Start.GetAttachParent();
	while (Parent != nullptr)
	{
		if (Parent->IsA<T>())
		{
			return Cast<T>(Parent);
		}
		Parent = Parent->GetAttachParent();
	}
	return nullptr;
}

template <typename UDerived, typename UBaseContainer>
TArray<UDerived*> FAGX_ObjectUtilities::Filter(const UBaseContainer& Collection)
{
	using UBase = typename std::remove_pointer<typename UBaseContainer::ElementType>::type;
	TArray<UDerived*> Result;
	for (UBase* Element : Collection)
	{
		if (UDerived* Match = Cast<UDerived>(Element))
		{
			Result.Add(Match);
		}
	}
	return Result;
}

template <typename T>
uint32 FAGX_ObjectUtilities::GetNumComponentsInActor(
	const AActor& Actor, bool bIncludeFromChildActors)
{
	TArray<T*> Components;
	Actor.GetComponents<T>(Components, bIncludeFromChildActors);
	return Components.Num();
}

template <typename T>
T* FAGX_ObjectUtilities::Get(const FComponentReference& Reference, const AActor* FallbackOwner)
{
	// Search among the Properties.
	// const_cast because FComponentReference require a non-const AActor. For now we assume that's
	// ok.
	AActor* FallbackOwner_ = const_cast<AActor*>(FallbackOwner);
	UActorComponent* ActorComponent = Reference.GetComponent(FallbackOwner_);
	if (T* Component = Cast<T>(ActorComponent))
	{
		// Found a Component of the correct type in a Property with the correct name.
		return Component;
	}

	// Search among all Components with the correct type.
	const AActor* SearchActor = FAGX_ObjectUtilities::GetActor(Reference, FallbackOwner);
	if (SearchActor == nullptr)
	{
		return nullptr;
	}
	TArray<T*> AllComponents;
	SearchActor->GetComponents(AllComponents, false);
	T** It = AllComponents.FindByPredicate(
		[Reference](T* Component) { return Component->GetFName() == Reference.ComponentProperty; });
	if (It == nullptr)
	{
		return nullptr;
	}
	return *It;
}

template <typename T>
TArray<T*> FAGX_ObjectUtilities::GetArchetypeInstances(T& Object)
{
	TArray<T*> Arr;
	if (!Object.HasAnyFlags(RF_ArchetypeObject))
	{
		return Arr;
	}

	TArray<UObject*> ArchetypeInstances;
	Object.GetArchetypeInstances(ArchetypeInstances);
	for (UObject* Obj : ArchetypeInstances)
	{
		T* Instance = Cast<T>(Obj);
		if (Instance == nullptr)
		{
			continue;
		}

		Arr.Add(Instance);
	}

	return Arr;
}
