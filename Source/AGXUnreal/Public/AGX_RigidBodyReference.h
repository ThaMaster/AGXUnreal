#pragma once

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

class AActor;

/**
 * A reference to an UAGX_RigidBodyComponent.
 *
 * The intention is that it should be used much like Actor pointers can be, but limitations in the
 * Unreal Editor forces us to do some tricks and workarounds. There is no Component picker, so the
 * user must first pick an Actor that owns the Component and then select the wanted component from a
 * combo box of body names. Except for the cache, there is no actual pointer to the Component stored
 * in the FAGX_RigidBodyReference, only the name, so renaming the body will break the reference.
 * This is a serious limitation. Also, while building a Blueprint Actor in the Blueprint editor
 * there is no actual Actor yet, so the Actor picker cannot be used to select the Actor that will be
 * created when the Blueprint is instantiated. For this reason all Components that include a
 * RigidBodyReference should set OwningActor to GetTypedOuter<AActor>() in PostInitProperties.
 *
 * void UMyComponent::PostInitProperties()
 * {
 *  	Super::PostInitProperties();
 *  	MyRigidBodyReference.OwningActor = GetTypedOuter<AActor>();
 * }
 *
 * This establishes the so-called local scope for the reference. Unless another OwningActor is
 * specified, the reference will search within the Actor that the Component is contained within. The
 * OwningActor set in PostInitProperties will we overwritten by deserialization if the object is
 * created from something else, such as part of a Play-in-Editor session or loaded from disk as part
 * of a cooked build.
 *
 * The RigidBodyReference supports caching of the RigidBodyComponent through the
 * CacheCurrentRigidBody member function. Only call this once the RigidBodyReference has been fully
 * formed, i.e., the OwningActor property set to the final Actor and when the referenced
 * RigidBodyComponent has been given its final name. BeginPlay is often a good choice.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference
{
	GENERATED_BODY()

	/**
	 * That Actor that owns the RigidBodyComponent that this RigidBodyReference references.
	 *
	 * The EditInstanceOnly specifier is set because Actor Blueprints should be self-contained,
	 * all Actor references should point to the Actor that is created when the Blueprint is
	 * instantiated in a level, which is achieved by not allowing a Blueprint to change the
	 * OwningActor, which means that the GetTypedOuter<AActor>() fetched in PostInitProperties will
	 * be the active OwningActor when instantiation is complete.
	 *
	 * Rules for OwningActor:
	 *  - The OwningActor should be set to GetTypedOuter<AActor>() in PostInitProperties of the
	 *    Component containing the RigidBodyReference to enable local scope lookup by default.
	 *  - The OwningActor should be set to GetTypedOuter<AActor>() in PostEditChangeProperty of the
	 *    Component containing the RigidBodyReference whenever OwningActor is changed to nullptr.
	 *    This reenables the local scope state.
	 */
	UPROPERTY(
		EditInstanceOnly, Category = "AGX Dynamics",
		Meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	AActor* OwningActor;

	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		Meta = (Tooltip = "The name of the RigidBodyComponent."))
	FName BodyName;

	/// \todo It may be possible to do this with a UAGX_RigidBodyComponent
	/// property instead of the name. The idea is to have a PropertyChanged
	/// callback that populate a combobox with all the UAGX_RigidBodyComponents
	/// in the Actor whenever the Actor property is changed.

	/// If true, then search not only within OwningActor's Components, but also recursively through
	/// ChildActorComponents as well.
	UPROPERTY(EditAnywhere, Category = "Body reference")
	uint8 bSearchChildActors : 1;

	/**
	 * Returns the cached body if there is one. Otherwise, performs a search to find and return the
	 * RigidBody that this reference currently references. Can return nullptr. Will return nullptr
	 * if OwningActor is nullptr or if OwningActor, and all of its child Actors if
	 * bSearchChildActors is true, does not contain a UAGX_RigidBodyComponent named BodyName.
	 *
	 * @return The UAGX_RigidBodyComponent that this FAGX_RigidBodyReference currently references.
	 */
	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Get the Actor that owns the RigidBody that this reference currently references. Can return
	 * nullptr. Can return non-null even if no RigidBody is currently the target of this reference.
	 * In that case the returned Actor is the Actor that will be searched the next time GetRigidBody
	 * is called.
	 */
	AActor* GetOwningActor() const;

	/**
	 * Forget the currently cached body, if any, and perform a new search through OwningActor's
	 * components. The found RigidBody will be cached and future calls to GetRigidBody will return
	 * the cached body even if the body is renamed or removed, or if OwningActor or BodyName is
	 * changed, until either a new call to CacheCurrentRigidBody or InvalidateCache is made.
	 */
	void CacheCurrentRigidBody();

	/**
	 * Forget the currently cached body. Should be called when any of the search
	 * parameters is changed and when the referenced body is renamed.
	 */
	void InvalidateCache();

private:
	UAGX_RigidBodyComponent* Cache;
};
