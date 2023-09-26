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
	static T* GetComponentByName(const AActor& Actor, const TCHAR* Name);

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

#if WITH_EDITOR
	/**
	 * Get the first Actor in the level that has the given label.
	 *
	 * The label is the name that is displayed in the World Outliner. Not that this may not be
	 * unique.
	 */
	static AActor* GetActorByLabel(const UWorld& World, const FString Label);
#endif

	static AActor* GetActorByName(const UWorld& World, const FString Name);

	/*
	 * Returns any Archetype instances of the passed object. If the passed object is not an
	 * archetype, an empty TArray is returned.
	 * Looks recursively in the archetype tree and returns all Instances in it.
	 * Note that an Archetype Instance may itself be an Archetype.
	 */
	template <typename T>
	static TArray<T*> GetArchetypeInstances(T& Object);

	/**
	 * Finds archetype instance with the given outer object. If the TemplateComponent has Outer as
	 * its outer, then TemplateComponent is returned. If non could be found, nullptr is returned.
	 */
	template <typename T>
	static T* GetMatchedInstance(T* TemplateComponent, UObject* Outer);

#if WITH_EDITOR
	/**
	 * Saves (or re-saves) an asset to disk. The asset must have a valid Package setup before
	 * passing it to this function.
	 * Setting FullyLoad to true will call Package->FullyLoad after save. This has been noted
	 * necessary when building cooked build on Linux in some situations.
	 */
	static bool SaveAsset(UObject& Asset, bool FullyLoad = false);
#endif

	/**
	 * Get the transform of any Component, even template Components residing in a Blueprint.
	 */
	static FTransform GetAnyComponentWorldTransform(const USceneComponent& Component);

	/**
	 * Set the transform of any Component, even template Components residing in a Blueprint. If the
	 * Component resides in a Blueprint and is a Component template, any archetype instances
	 * currently "in sync" with the Component will be updated as well. The archetype instances
	 * update only happens in editor builds.
	 */
	static void SetAnyComponentWorldTransform(
		USceneComponent& Component, const FTransform& Transform,
		bool ForceOverwriteInstances = false);

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
T* FAGX_ObjectUtilities::GetComponentByName(const AActor& Actor, const TCHAR* Name)
{
	// The Components are stored in a TSet but I don't know how to search a TSet with a predicate
	// So copying all the pointers to a TArray. Is there a better way?
	//
	// That question can be asked in general, this seems like a complicated way to find a Component
	// in an Actor.
	TArray<T*> Components;
	Actor.GetComponents(Components);
	auto It = Components.FindByPredicate([Name](const T* Component)
										 { return Component->GetName() == Name; });
	return It ? *It : nullptr;
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

template <typename T>
T* FAGX_ObjectUtilities::GetMatchedInstance(T* TemplateComponent, UObject* Outer)
{
	if (TemplateComponent == nullptr || Outer == nullptr)
		return nullptr;

	if (TemplateComponent->GetOuter() == Outer)
		return TemplateComponent;

	for (auto Instance : GetArchetypeInstances(*TemplateComponent))
	{
		if (Instance->GetOuter() == Outer)
		{
			return Instance;
		}
	}

	return nullptr;
}

// clang-format off
#define AGX_COPY_PROPERTY_FROM(UpropertyName, GetterExpression, Component, ForceOverwriteInstances) \
{ \
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component)) \
	{ \
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component)) \
		{ \
			if (ForceOverwriteInstances || Instance->UpropertyName == (Component).UpropertyName) \
			{ \
				Instance->UpropertyName = GetterExpression; \
			} \
		} \
	} \
	(Component).UpropertyName = GetterExpression; \
}
// clang-format on
