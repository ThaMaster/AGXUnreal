// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "AGX_ComponentReference.generated.h"

class UActor;
class UActorComponent;

/**
 * A reference to a typed Component by name.
 *
 * The Component is identified by an Owning Actor pointer and the name of the Component. The type
 * is specified using a UClass, specifically TSubclassOf<UActorComponent>. Instances are meant to be
 * created as members of C++ classes, but is usable also from Blueprint Visual Scripts.
 *
 * The intention is that it should be used much like Actor pointers can be, but limitations in the
 * Unreal Editor with Components forces us to do some tricks and workarounds. There is no Component
 * picker, so the user must first pick an Actor that owns the Component and then select the wanted
 * component from a combo box of available names. There is no actual pointer to the Component stored
 * in the FAGX_ComponentReference, only the name, so renaming the component will break the
 * reference. This is a serious limitation. Also, while building a Blueprint Actor in the Blueprint
 * editor there is no actual Actor yet, so the Actor picker cannot be used to select the Actor that
 * will be created when the Blueprint is instantiated. For this reason all Components that include a
 * Component Reference should set OwningActor to GetTypedOuter<AActor>() in PostInitProperties.
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
 * created from something else, such as part of a Play-in-Editor session start-up or loaded from
 * disk as part of a cooked build start-up.
 *
 * Unreal Editor detects changes made during construction, PostInitProperties is considered part of
 * the construction phase, and will disable editing of UProperties with such changes. This can be
 * disabled by adding the SkipUCSModifiedProperties Meta Specifier to the UProperty. That should be
 * done for FAGX_ComponentReferences on which OwningActor is set during PostInitProperties, and
 * recursively up any struct holding the FAGX_ComponentReference until a UObject UProperty is
 * reached.
 *
 *   UPROPERTY(EditAnywhere, Category = "My Category", Meta = (SkipUCSModifiedProperties))
 *   FAGX_ComponentReference MyComponentReference;
 *
 *   UPROPERTY(EditAnywhere, Category = "My Category", Meta = (SkipUCSModifiedProperties))
 *   FStructContainingComponentReference MyNestedComponentReference;
 *
 * There are multiple sub-classes of FAGX_ComponentReference that specify the more specific
 * Component type, for example FAGX_RigidBodyReference and FAGX_ShovelReference.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_ComponentReference();
	FAGX_ComponentReference(TSubclassOf<UActorComponent> InComponentType);

	UPROPERTY(
		EditInstanceOnly, BlueprintReadWrite, Category = "AGX Component Reference",
		Meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	AActor* OwningActor {nullptr};

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
