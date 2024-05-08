// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "AGX_ComponentReference.generated.h"

class UActor;
class UActorComponent;

/**
 * A reference to a typed Component by name and owning Actor.
 *
 * The Component is identified by an Owning Actor pointer and the name of the Component. The type
 * is specified using a UClass, specifically TSubclassOf<UActorComponent>. Instances are meant to be
 * created as members of C++ classes, but the type is usable also from Blueprint Visual Scripts.
 *
 * There are multiple sub-classes of FAGX_ComponentReference that specify the more specific
 * Component type, for example FAGX_RigidBodyReference and FAGX_ShovelReference.
 *
 * The intention is that the Component Reference should be used much like Actor pointers are, but
 * limitations in the Unreal Editor with Components forces us to do some tricks and workarounds.
 * There is no Component picker UI, so the user must first pick an Actor that owns the Component and
 * then select the wanted component from a combo box of available names. There is no actual pointer
 * to the Component stored in the FAGX_ComponentReference, only the name, so renaming the component
 * will break the reference. This is a serious limitation. Also, while building a Blueprint Actor in
 * the Blueprint editor there is no actual Actor yet, so the Actor picker cannot be used to select
 * the Actor that will be created when the Blueprint is instantiated. In fact it cannot reference
 * any Actor. For this reason the Component Reference contains a Local Scope non-Property member
 * that is used as a fallback search Actor when Owning Actor is None / nullptr. All Components that
 * contains a Component Reference member variable should set Local Scope to the Actor that it is
 * part of in the constructor:
 *
 * void UMyComponent::UMyComponent()
 * {
 *  	MyComponentReference.SetLocalScope(GetTypedOuter<AActor>());
 * }
 *
 * This establishes the so-called local scope for the reference. Unless an Owning Actor is
 * specified, the reference will search within the Actor that the reference is contained within. If
 * the Component contains Component References that are created dynamically, such as in an array,
 * then the Local Scope should be set on those in Post Load since at that point any array elements
 * found in serialized data has been restored. If a Blueprint creates elements in the array in its
 * Construction Script then the Local Scope on those must be set in On Register. On Register is also
 * called after elements has been added to an array from the Details panel. If the Component
 * provide member functions that adds or sets elements in the array then those functions should set
 * the Local Scope on the added or set element.
 *
 * A struct that both contains an FAGX_ComponentReference and has custom serialization code must
 * ensure that the garbage collector is made aware of the possible change in referencing the Owning
 * Actor. This is done by calling SerializeTaggedProperties also when
 * IsModifyingWeakAndStrongReferences is true. See FAGX_WireRoutingNode for an example or below
 * for the basics, and this Unreal Developer Network question:
 * https://udn.unrealengine.com/s/question/0D5QP000008jPSZ0A2/uproperty-aactor-target-set-to-nullptr-by-garbage-collector-after-compile-of-targets-blueprint-class
 *
 * bool FMyStruct::Serialize(FArchive& Archive)
 * {
 *     // Serialize the normal UPROPERTY data.
 *     if (Archive.IsLoading()
 *	       || Archive.IsSaving()
 *	       || Archive.IsModifyingWeakAndStrongReferences())
 *     {
 *         UScriptStruct* Struct = FMyStruct::StaticStruct();
 *         Struct->SerializeTaggedProperties(
 *             Archive, reinterpret_cast<uint8*>(this), Struct, nullptr);
 *     }
 *
 *     // Struct-specific serialization code goes here.
 * }
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_ComponentReference();
	FAGX_ComponentReference(TSubclassOf<UActorComponent> InComponentType);

	// It would be safer to implement the copy constructor and assignment operator to not copy Local
	// Scope, so that we can be sure we don't ever copy a Component Reference from one Component
	// into another without updating Local Scope. Unfortunately, Blueprint Visual Script doesn't
	// support references to structs, it will always copy from C++ to Visual Script, which means
	// that if we do not copy Local Scope then there is no way of getting a Component that is
	// referenced within the local scope. So we must copy.

	UPROPERTY(
		EditInstanceOnly, BlueprintReadWrite, Category = "AGX Component Reference",
		Meta = (Tooltip = "The Actor that owns the Component."))
	AActor* OwningActor {nullptr};

	/**
	 * Actor searched if Owning Actor is nullptr or points to an invalid Actor. Should always
	 * be set in the constructor of the Component that contains this Component Reference.
	 *
	 * Intentionally not a UProperty because we do not want this to be serialized or used in equals
	 * comparisons. We want the Component that contains this Component Reference to always be in
	 * control of this pointer.
	 */
	AActor* LocalScope {nullptr};

	/**
	 * Let the Component Reference know which Actor is the local scope. If the given Actor is a
	 * Child Actor then the parent actor chain will be traversed until a non-Child-Actor is found.
	 */
	void SetLocalScope(AActor* InLocalScope);

	/**
	 * Get the Actor that should be searched for the referenced Component. Is Owning Actor if that
	 * is valid, if not then Local Scope is returned.
	 */
	AActor* GetScope() const;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Component Reference",
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
	TArray<UActorComponent*> GetCompatibleComponents() const;

	/// The type of Component that this Reference is allowed to reference.
	TSubclassOf<UActorComponent> ComponentType;

	// @todo Should we add caching? Is it safe, even in the face of Blueprint Reconstruction? Can
	// we find cases where it is guaranteed to be safe?
	// Is the Unreal Engine F(Base)ComponentReference type using caching?
	// Can we scrap this custom FAGX_ComponentReference and use FComponentReference yet?
	// See internal issue 466 and UDN thread
	// https://udn.unrealengine.com/s/question/0D54z000073a3w8CAA/how-do-one-use-usecomponentpicker-on-fcomponentreference.
	// Old comment about this:
	/*
	 * The ComponentReference supports caching of the RigidBodyComponent through the
	 * CacheCurrentComponentBody member function. Only call this once the RigidBodyReference has
	 * been fully formed, i.e., the OwningActor property set to the final Actor and when the
	 * referenced RigidBodyComponent has been given its final name. BeginPlay is often a good
	 * choice. Though beware that if the target Rigid Body is destroyed due to a Blueprint
	 * Reconstruction then the cache becomes a dangling pointer.
	 */
	// Another old comment:
	/*
	 * The following comment describe tha challenges of using caching in Component references.
	 * When we replaced the specialized Rigid Body Reference with the more general Component
	 * reference we did not port over caching. The contents of this comment should be considered
	 * if we ever decide to introduce caching again.
	 *
	 * This may be complicated. Normally, all the Components of an Actor exists when the first
	 * BeginPlay is called. Under those conditions it is possible to cache the the Component
	 * pointers.
	 *
	 * However, when editing Properties in the Details Panel during a Play In Editor session
	 * Unreal Engine creates and initializes one component at the time. Which means that this
	 * Constraint may get BeginPlay before the Rigid Body it is attached to even exists. So
	 * caching in BeginPlay is not possible while the GIsReconstructingBlueprintInstances flag
	 * is set.
	 *
	 * Not sure where else to do it. I don't know of any later startup callback. Should we skip
	 * caching all together, and do a search every time we need the Component? Should we use the
	 * OldToNew map passed to Component Instance Data to update the pointers? Set a flag in the
	 * RigidBody/Component Reference to indicate that it is allowed to cache the next time a
	 * look-up is done?
	 *
	 * Things are made even more complicated if the Rigid Body or Component we're referencing
	 * is in another Blueprint and that Blueprint is being recreated. Nothing in that Blueprint
	 * knows that this Constraint needs to be notified about the reconstruction.
	 */
};

inline bool operator==(const FAGX_ComponentReference& Lhs, const FAGX_ComponentReference& Rhs)
{
	return Lhs.OwningActor == Rhs.OwningActor
		   // Intentionally not comparing Local Scope.
		   && Lhs.Name == Rhs.Name && Lhs.bSearchChildActors == Rhs.bSearchChildActors;
}

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
