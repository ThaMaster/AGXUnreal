// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_Utilities.generated.h"

class UAGX_RigidBodyComponent;

/**
 * Helper type that calls the given callback when destructed. Useful when cleanup is needed
 * regardless of from where we return from a function or leave a scope.
 * @tparam FuncT Callback to call on destruction. Will be moved from.
 */
template <typename FuncT>
struct FAGX_Finalizer
{
	FAGX_Finalizer(FuncT InCallback)
		: Callback(std::move(InCallback))
	{
	}

	~FAGX_Finalizer()
	{
		Callback();
	}

	FuncT Callback;
};

UCLASS(ClassGroup = "AGX Utilities")
class AGXUNREAL_API UAGX_AGXUtilities : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

	/**
	 * Add the Parent velocity/angular velocity to the Body's velocity as if rigidly attached.
	 * This function is only valid during Play and does nothing if not in Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static void AddParentVelocity(UAGX_RigidBodyComponent* Parent, UAGX_RigidBodyComponent* Body);

	/**
	* Add the Parent velocity/angular velocity to all Bodies velocities as if rigidly attached.
	* If the Parent body is among the provided Bodies array, it's velocity will not be updated.
	* This function is only valid during Play and does nothing if not in Play.
	*/
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static void AddParentVelocityMany(
		UAGX_RigidBodyComponent* Parent, const TArray<UAGX_RigidBodyComponent*>& Bodies);
};
