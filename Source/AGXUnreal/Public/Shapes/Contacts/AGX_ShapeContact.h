#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/Contacts/ShapeContactData.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

class UAGX_ShapeComponent;
class UAGX_RigidBodyComponent;

#include "AGX_ShapeContact.generated.h"

/**
 * TODO: add description
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ShapeContact
{
	GENERATED_USTRUCT_BODY()

	FAGX_ShapeContact(FShapeContactData&& InData) noexcept
		: Data(std::move(InData))
	{
	}

	FAGX_ShapeContact() = default;


private :
	friend class UAGX_ShapeContact_FL;
	FShapeContactData Data;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
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
	 * Return nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetFirstBody(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	/**
	 * Return nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static UAGX_RigidBodyComponent* GetSecondBody(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static int GetNumPoints(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointPosition(
		UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointForce(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointNormalForce(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static FVector GetPointNormal(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static float GetPointDepth(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	static float GetPointArea(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex);
};
