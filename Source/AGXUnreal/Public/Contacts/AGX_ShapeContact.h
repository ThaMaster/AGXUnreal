// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactPoint.h"
#include "Contacts/ShapeContactBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/EmptyShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ShapeContact.generated.h"

class UAGX_ShapeComponent;
class UAGX_RigidBodyComponent;

/**
 * Struct that holds contact data for a shape. Note that this data is only valid during a single
 * simulation step.
 */
USTRUCT(Category = "AGX", BlueprintType)
struct AGXUNREAL_API FAGX_ShapeContact
{
	GENERATED_BODY()

	FAGX_ShapeContact() = default;
	FAGX_ShapeContact(const FShapeContactBarrier& InBarrier);
	FAGX_ShapeContact(FShapeContactBarrier&& InBarrier);

	/**
	 * @return True if this ShapeContact is backed by native AGX Dynamics data.
	 */
	bool HasNative() const;

	/**
	 * @return True if the Shape Contact is enabled, i.e., its ContactPoints are included in the
	 * upcoming solve.
	 */
	bool IsEnabled() const;

	FRigidBodyBarrier GetBody1() const;
	FRigidBodyBarrier GetBody2() const;

	FEmptyShapeBarrier GetShape1() const;
	FEmptyShapeBarrier GetShape2() const;

	/**
	 * @param PointIndex The index of the contact point to calculate the relative velocity at.
	 * @return The relative velocity at the contact point, in the world coordinate system.
	 */
	FVector CalculateRelativeVelocity(int32 PointIndex) const;

	int32 GetNumContactPoints() const;

	TArray<FAGX_ContactPoint> GetContactPoints() const;

	FAGX_ContactPoint GetContactPoint(int32 Index) const;

private:
	FShapeContactBarrier Barrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_ShapeContact in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ShapeContact_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Determine if the Shape Contact represents a valid AGX Dynamics contact or not. No functions
	 * should be called on it if there is no native AGX Dynamics Shape Contact.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool HasNative(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Determine if the Shape Contact is enabled, i.e. whether the Contact Points it contains should
	 * be seen by the solver.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool IsEnabled(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the first, of two, Shape that collided.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_ShapeComponent* GetFirstShape(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the second, of two, Shape that collided.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_ShapeComponent* GetSecondShape(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the first, of two, Rigid Body that collided.
	 *
	 * Returns None / nullptr if the first Shape does not have a Rigid Body among the attachment
	 * ancestors.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetFirstBody(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the second, of two, Rigid Body that collided.
	 *
	 * Returns None / nullptr if the second Shape does not have a Rigid Body among the attachment
	 * ancestors.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetSecondBody(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the number of contact points in the Shape Contacts.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static int32 GetNumContactPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the number of contact points in the Shape Contacts.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static int32 GetNumPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Returns the last valid point index. Returns -1 if there are no points and thus no valid
	 * index.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static int32 GetLastPointIndex(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Determine if the contact point represents a valid AGX Dynamics Contact Point or not. No
	 * functions should be called on it if there is not native AGX Dynamics Contact Point.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool HasPointNative(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Determine is the Contact Point is enabled, i.e. whether the Contact Point should be seen by
	 * the solver. Only Contact Points that are part of an enabled Shape Contact is seen by the
	 * solver.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool IsPointEnabled(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);


	/**
	 * Get the intersection depth between the two colliding Shapes [cm].
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static float GetPointDepth(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the world location of the Contact Point [cm].
	 *
	 * The location specifies where the solver will apply contact forces onto the Rigid Bodies
	 * involved in the contact.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointLocation(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the normal direction of the Contact Point.
	 *
	 * The normal specifies the direction in which the solver will apply normal forces from the
	 * contact point.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointNormal(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);


	/**
	 * Get the first, of two, tangent direction of the Contact Point.
	 *
	 * The tangent specifies the direction in which the solver will apply friction forces from the
	 * Contact Point.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointTangentU(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the second, of two, tangent direction of the Contact Point.
	 *
	 * The tangent specifies the direction in which the solver will apply friction forces from the
	 * Contact Point.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointTangentV(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the total contact force, in world coordinates. Includes both normal and friction forces.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointForce(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the normal force of the contact, in world coordinates.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointNormalForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the tangential, i.e. friction, force of the contact.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointTangentialForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get all contact force components. Ordered normal, tangent U, tangent V.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointLocalForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Get the witness point for either of the Shapes involved in the contact. The witness point is
	 * a point on the surface of the Shape in the direction of the normal from the contact point
	 * location.
	 *
	 * Shape Index should be either 0 or 1.
	 *
	 * @param ShapeIndex 0 to get the witness point on the first shape, 1 to get the witness point
	 * on the second shape.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointWitnessPoint(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, int32 ShapeIndex);

	/**
	 * Get the area represented by the contact point.
	 *
	 * Only relevant if Use Contact Area Approach is enabled on the Contact Material.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static float GetPointArea(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Compute the relative velocity at the given contact point. Will be zero for a stable contact.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector CalculateRelativeVelocity(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);
};
