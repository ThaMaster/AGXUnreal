#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/Sensors/SensorContactData.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

class UAGX_ShapeComponent;
class UAGX_RigidBodyComponent;

#include "AGX_SensorContact.generated.h"

/**
 * TODO: add description
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_SensorContact
{
	GENERATED_USTRUCT_BODY()

	FAGX_SensorContact(FSensorContactData&& InData) noexcept
		: Data(std::move(InData))
	{
	}

	FAGX_SensorContact() = default;


private :
	friend class UAGX_SensorContact_FL;
	FSensorContactData Data;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_SensorContact_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static UAGX_ShapeComponent* GetFirstShape(UPARAM(ref) FAGX_SensorContact& SensorContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static UAGX_ShapeComponent* GetSecondShape(UPARAM(ref) FAGX_SensorContact& SensorContactRef);

	/**
	 * Return nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static UAGX_RigidBodyComponent* GetFirstBody(UPARAM(ref) FAGX_SensorContact& SensorContactRef);

	/**
	 * Return nullptr if the Shape does not have a RigidBody as parent.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static UAGX_RigidBodyComponent* GetSecondBody(UPARAM(ref) FAGX_SensorContact& SensorContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static int GetNumPoints(UPARAM(ref) FAGX_SensorContact& SensorContactRef);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static FVector GetPointPosition(
		UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static FVector GetPointForce(UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static FVector GetPointNormalForce(UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static FVector GetPointNormal(UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static float GetPointDepth(UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Contacts")
	static float GetPointArea(UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex);
};
