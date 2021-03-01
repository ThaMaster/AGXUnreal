#pragma once

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

class AActor;

/**
 * A reference to a UAGX_RigidBodyComponent.
 *
 * The intention is that it should be used much like Actor pointers can be, but limitations in the
 * Unreal Editor forces us to do some tricks and workarounds. There is no Component picker, so the
 * user must first pick an Actor that owns the Component and then select the wanted component from a
 * combo box of body names. There is no actual pointer to the Component stored in the
 * FAGX_RigidBodyReference, only the name, so renaming the body will break the reference. This is a
 * serious limitation. Also, while building a Blueprint Actor in the Blueprint editor there is no
 * actual Actor yet, so the Actor picker cannot be used to select the Actor that will be created
 * when the Blueprint is instantiated. The RigidBodyReference provides a Fallback Owning Actor for
 * this purpose. Having Owning Actor set to nullptr and Fallback Owning Actor non-nullptr signals
 * that Component lookup should be done in the Fallback Owning Actor, and that the Component that
 * contains the UAGX_RigidBodyReference should clear the Fallback Owning Actor pointer and set the
 * Owning Actor pointer once the Blueprint has been instantiated and the actual Actor instance is
 * known. In short, nullptr Owning Actor and non-nullptr Fallback Owning Actor means "search in the
 * local scope and finalize the Owning Actor pointer as soon as possible".
 *
 * The RigidBodyReference supports caching of the RigidBodyComponent through the
 * CacheCurrentRigidBody member function. Only call this once the RigidBodyReference has been fully
 * formed, i.e., the OwningActor property set to the final Actor and when the referenced
 * RigidBodyComponent has been given its final name.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference
{
	GENERATED_BODY()

	// Using a SoftObjectPtr instead of a raw pointer because we use FAGX_RigidBodyReference in the
	// AGXUnreal Mode for constraint creation and Unreal Engine does not allow raw pointers to world
	// objects being held by non-world objects. The constraint creation Mode is a non-world object.
	// When used in a constraint that exists in the level the OwningActor should always point to an
	// Actor in the same world as the constraint.
	//
	// A LazyObjectPtr could be used as well, I think. I don't know which of the two would be better
	// in this case.
	//
	// The EditInstanceOnly specifier is passed because Actor Blueprints should be self-contained
	// so all Actor references should implicitly point to the Actor that will be created when the
	// Blueprint is instantiated in a level. I can imagine cases where the user want to create a
	// Blueprint that is bound to a particular level and uses references to objects within the
	// level, but that will not be supported at this stage.

	// New rules after AGXUNREAL_RIGID_BODY_REFERENCE_REFACTOR:
	//  - There is only one Owning Actor pointer.
	//  - The Fallback Owning Actor pointer has been removed.
	//  - The Owning Actor should be set to GetTypedOuter<AActor> in the constructor of the
	//    Component containing the Rigid Body Reference.
	//  - Owning Actor pointing to GetTypedOuter<AActor> is semantically equivalent to the old way
	//    of setting Owning Actor to nullptr and the FallbackOwningActor to GetOwner().
	//  - By setting Owning Actor in the constructor, instead of PostLoad, we expect the property
	//    to renain editable in the Details Panel.
	//  - We expect the Owning Actor pointer to be correct both after Blueprint instantiation and
	//    after Play instantiation.
	//  - After Blueprint instantiation because the Component instance will set the Owning Actor
	//    to GetTypedOuter<AActor>() in the constructor, which is correct in that case.
	//  - After BeginPlay instantiation because the source Component will either not have
	//    overwritten the Owning Actor, in which case the Play instance will keep its
	//    GetTypedOuter<AActor>() Owning Actor set in its constructor; or the source component have
	//    overwritten its own Owning Actor, in which case the Play instance will get the reference
	//    corrected version of that Owning Actor since Owning Actor is a UPROPERTY pointer of a type
	//    (AActor) for which reference correction is performed by Unreal Engine.
	//  - We should needless trickery with Owning Actor in PostEditChange, PostLoad, or
	//    BeginPlay.
	//    Trickeries performed previously:
	//     - Containing Component's PostEditChangeProperty, Owning Actor became nullptr:
	//       Set Fallback Owning Actor to GetOwner().
	//       This switches the reference to local scope mode.
	//     - Containing Component's PostLoad:
	//       This event marks the finalization of a Play object.
	//       Set Fallback Owning Actor to nullptr to disable fallback mode.
	//       If Owning Actor is nullptr, set it to GetOwner().
	//     - FAGX_ConstraintIconGraphicsProxy constructor:
	//       If Owning Actor is nullptr, set Fallback Owner Actor to GetOwner().
	//       I assume this is just a way to ensure a proper fallback setup in case the other
	//       Fallback Owning Actor setting code hasn't been run yet.
	//     - UAGX_TwoBodyTireComponent constructor:
	//       Set Fallback Owning Actor to GetOwner(). I'm not sure if this even works. Has the Owner
	//       already been set here?
	//     - UAGX_TwoBodyTireComponent::PostLoad:
	//       This is an instance of "Containing Component's PostLoad" above.
	//     - FAGX_ArchiveImporterHelper::InstantiateTwoBodyTire:
	//       Set Owning Actor to the Body's GetOwner() for non-Blueprint imports.
	//     - AGX_ArchiveImporterToBlueprint::InstantiateConstraint:
	//       Set OwningActor to nullptr.
	//    New list of trickeries:
	//     - Containing Component's PostEditChangeProperty, Owning Actor became nullptr:
	//       Set Owning Actor to GetTypedOuter<AActor>().
	//       This resets Owning Actor back to its default state, what was set in the constructor.
	//     - Containing Component's PostLoad: No action required.
	//     - FAGX_ConstraintIconGraphicsProxy constructor: No action required.
	//     - UAGX_TwoBodyTireComponent constructor: No action required.
	//     - UAGX_TwoBodyTireComponent::PostLoad: No action required.
	//     - FAGX_ArchiveImporterHelper::InstantiateTwoBodyTire:
	//       Set Owning Actor to the Body's GetOwner().
	//     - AGX_ArchiveImporterToBlueprint::InstantiateConstraint: No action required.
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		meta = (Tooltip = "The Actor that owns the RigidBodyComponent."))
	AActor* OwningActor;

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	FName BodyName;

	/// \todo It may be possible to do this with a UAGX_RigidBodyComponent
	/// property instead of the name. The idea is to have a PropertyChanged
	/// callback that populate a combobox with all the UAGX_RigidBodyComponents
	/// in the Actor whenever the Actor property is changed.

	/// If true, then search not only within OwningActor's Components, but also recursively through
	/// ChildActorComponents as well.
	UPROPERTY(EditAnywhere, Category = "Body reference")
	uint8 bSearchChildActors : 1;

	// A fallback owning Actor that can be used when we want to specify an Actor to search for
	// RigidBodies in but don't want to expose it to the user and don't want to accidentally leak
	// the Actor pointer into an actual game session. Used, for example, while building Blueprint
	// Actors in the Blueprint editor during which the FallbackOwningActor points to the Blueprint
	// itself.
	//
	// FallbackOwningActor is ignored while OwningActor is not nullptr.
	// Should be cleared on BeginPlay in the object that holds this RigidBodyReference.
	// Intentionally not a UPROPERTY.
	// AActor* FallbackOwningActor;

	/**
	 * Returns the cached body if there is one. Otherwise, performs a search to find and return the
	 * RigidBody that this reference currently references. Can return nullptr. Will return nullptr
	 * if both OwningActor and FallbackOwningActor is nullptr and if neither of them, and all of
	 * their child actors if bSearchChildActors is true, contain a UAGX_RigidBodyComponent named
	 * BodyName.
	 *
	 * FallbackOwningActor will not be searched if OwningActor is non-nullptr even if no matching
	 * RigidBody is found in OwningActor.
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
	 * Forget the currently cached body, if any, and perform a new search through
	 * OwningActor's components. The found RigidBody will be cached and future calls to GetRigidBody
	 * will return the cached body even if the body is renamed or removed, or if OwningActor or
	 * BodyName is changed, until either a new call to CacheCurrentRigidBody or InvalidateCache is
	 * made.
	 *
	 * Will not search in FallbackOwningActor, only OwningActor.
	 *
	 * \todo The non-caching of FallbackOwningActor may make the Unreal Editor sluggish. We should
	 * convince ourselves that we can keep the cache correct.
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
