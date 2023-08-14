// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ComponentReference.generated.h"

class UActor;
class UActorComponent;

/**
 * A reference to a typed Component by name.
 *
 * The Component is identified by an Owning Actor pointer and the name of the Component. The type
 * specified using a UClass, specifically TSubclassOf<UActorComponent>. Instances are meant to be
 * created as members of C++ classes, but usable also from Blueprint Visual Scripts.
 *
 * The intention is that it should be used much like Actor pointers can be, but limitations in the
 * Unreal Editor with Components forces us to do some tricks and workarounds. There is no Component
 * picker, so the user must first pick an Actor that owns the Component and then select the wanted
 * component from a combo box of available names. There is no actual pointer to the Component
 * stored in the FAGX_ComponentReference, only the name, so renaming the component will break the
 * reference. This is a serious limitation. Also, while building a Blueprint Actor in the
 * Blueprint editor there is no actual Actor yet, so the Actor picker cannot be used to select the
 * Actor that will be created when the Blueprint is instantiated. For this reason all Components
 * that include a Component Reference should set OwningActor to GetTypedOuter<AActor>() in
 * PostInitProperties.
 *
 * void UMyComponent::PostInitProperties()
 * {
 *  	Super::PostInitProperties();
 *  	MyRigidBodyReference.OwningActor = GetTypedOuter<AActor>();
 * }
 *
 * This establishes the so-called local scope for the reference. Unless another OwningActor is
 * specified, the reference will search within the Actor that the reference is contained within. The
 * OwningActor set in PostInitProperties will we overwritten by deserialization if the object is
 * created from something else, such as part of a Play-in-Editor session or loaded from disk as part
 * of a cooked build.
 *
 * Unreal Editor detects changes made during construction, PostInitProperties is considered part of
 * the construction phase, and will disable editing of UProperties with such changes. This can be
 * disabled by adding the SkipUCSModifiedProperties Meta Specifier to the UProperty. That should be
 * done for FAGX_ComponentReferences on which OwningActor is set during PostInitProperties, and
 * recursively up any struct holding the FAGX_RigidBodyReference until a UObject UProperty is
 * reached.
 *
 *   UPROPERTY(EditAnywhere, Category = "My Category", Meta = (SkipUCSModifiedProperties))
 *   FAGX_ComponentReference MyRigidBodyReference;
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_ComponentReference();
	FAGX_ComponentReference(TSubclassOf<UActorComponent> InComponentType);

	UPROPERTY(
		EditInstanceOnly, Category = "AGX Component Reference",
		Meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	AActor* OwningActor {nullptr};

	UPROPERTY(
		EditAnywhere, Category = "AGX Component Reference",
		Meta = (Tooltip = "The name of the Component."))
	FName Name {NAME_None};

	UPROPERTY(
		EditAnywhere, Category = "AGX Component Reference",
		Meta =
			(Tooltip =
				 "Whether the search for the Component should decend into Child Actor Components."))
	uint8 bSearchChildActors : 1;

	/**
	 * Does a search in Owning Actor for a Component named Name. Will return nullptr if no matching
	 * Component is found.
	 *
	 * @return The Component that the Reference currently references. Can be nullptr.
	 */
	UActorComponent* GetComponent() const;

	/**
	 * @see GetComponent()
	 * @tparam T The type of the Component to get. Must match Component Type.
	 * @return The Component that the Reference currently references. Can be nullptr.
	 */
	template <typename T>
	T* GetComponent() const;

	/**
	 * Get a list of all Components in OwningActor that this reference is allowed to point to,
	 * based on set the Component Type.
	 * @param OutComponents The found Components.
	 */
	void GetCompatibleComponents(TArray<UActorComponent*>& OutComponents) const;

	/// The type of Component that this Reference is allowed to reference.
	TSubclassOf<UActorComponent> ComponentType;

	// @todo Should we add caching? Is it safe, even in the face of Blueprint Reconstruction? Can
	// we find cases where it is guaranteed to be safe?
	// Old comment about this.
	/*
	 * The ComponentReference supports caching of the RigidBodyComponent through the
	 * CacheCurrentComponentBody member function. Only call this once the RigidBodyReference has
	 * been fully formed, i.e., the OwningActor property set to the final Actor and when the
	 * referenced RigidBodyComponent has been given its final name. BeginPlay is often a good
	 * choice. Though beware that if the target Rigid Body is destroyed due to a Blueprint
	 * Reconstruction then the cache becomes a dangling pointer.
	 */
};

template <typename T>
T* FAGX_ComponentReference::GetComponent() const
{
	return Cast<T>(GetComponent());
}

UCLASS()
class AGXUNREAL_API UAGX_ComponentReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Component Reference")
	static UActorComponent* GetComponent(UPARAM(Ref) FAGX_ComponentReference& Reference);
};
