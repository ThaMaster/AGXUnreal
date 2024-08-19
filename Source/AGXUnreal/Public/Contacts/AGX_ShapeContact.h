// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/ShapeContactBarrier.h"
#include "Contacts/AGX_ContactPoint.h"
#include "Shapes/EmptyShapeBarrier.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

class UAGX_ShapeComponent;
class UAGX_RigidBodyComponent;

#include "AGX_ShapeContact.generated.h"

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
	 * solve.
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

	FAGX_ContactPoint GetContactPoint(int Index) const;

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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool HasNative(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool IsEnabled(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_ShapeComponent* GetFirstShape(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_ShapeComponent* GetSecondShape(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Returns nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetFirstBody(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Returns nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetSecondBody(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector CalculateRelativeVelocity(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static int32 GetNumContactPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static int32 GetNumPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Returns the last valid point index. Returns -1 if there are no points and thus no valid index.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static int32 GetLastPointIndex(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool HasPointNative(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static bool IsPointEnabled(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static float GetPointDepth(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointLocation(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointNormal(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointTangentU(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointTangentV(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointForce(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointNormalForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointTangentialForce(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointLocalForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static FVector GetPointWitnessPoint(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, int32 ShapeIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contacts")
	static float GetPointArea(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);
};
