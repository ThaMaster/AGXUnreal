// Copyright 2022, Algoryx Simulation AB.


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

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static UAGX_ShapeComponent* GetFirstShape(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static UAGX_ShapeComponent* GetSecondShape(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	/**
	 * Returns nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetFirstBody(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	/**
	 * Returns nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetSecondBody(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector CalculateRelativeVelocity(
		UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static int GetNumContactPoints(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static float GetPointDepth(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointLocation(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointForce(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointNormalForce(
		UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointNormal(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static float GetPointArea(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);
};
