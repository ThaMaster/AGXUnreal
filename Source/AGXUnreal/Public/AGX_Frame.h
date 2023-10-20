// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_Frame.generated.h"

class USceneComponent;

/**
 * Specifies a transformation relative to a Scene Component.
 *
 * Used, for example, to specify where something attaches to something else, or where something is.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_Frame
{
	GENERATED_BODY()

	FAGX_Frame();

	/**
	 * The Component that this Frame is relative to.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Frame")
	FAGX_ComponentReference Parent;

	/*
	 * Set the given Component to be the parent of this frame.
	 *
	 * Note that the raw pointer will not be stored, instead the owner of the Component and its name
	 * is stored which means that if the Component is renamed then the relationship is lost. It also
	 * means that the relationship will survive a Blueprint Reconstruction.
	 */
	void SetParentComponent(USceneComponent* Component);
	USceneComponent* GetParentComponent() const;

	/**
	 * The location of the origin of this Frame, specified in the Parent's local coordinate system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Frame")
	FVector LocalLocation {FVector::ZeroVector};

	/**
	 * The rotation of the origin of this Frame, specified in the Parent's local coordinate system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Frame")
	FRotator LocalRotation {FRotator::ZeroRotator};

	FVector GetWorldLocation() const;
	FVector GetWorldLocation(const USceneComponent& FallbackParent) const;

	FRotator GetWorldRotation() const;
	FRotator GetWorldRotation(const USceneComponent& FallbackParent) const;

	void GetWorldLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const;

	/**
	 * Get the location of this Frame relative to the given Scene Component.
	 */
	FVector GetLocationRelativeTo(const USceneComponent& Component) const;

	FVector GetLocationRelativeTo(
		const USceneComponent& Component, const USceneComponent& FallbackParent) const;

	/**
	 * Get the rotation of this Frame relative to the given Scene Component.
	 */
	FRotator GetRotationRelativeTo(const USceneComponent& Component) const;

	FRotator GetRotationRelativeTo(
		const USceneComponent& Component, const USceneComponent& FallbackParent) const;

	void GetRelativeTo(
		const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation) const;

	void GetRelativeTo(
		const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation,
		const USceneComponent& FallbackParent) const;
};

UCLASS()
class AGXUNREAL_API UAGX_Frame_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	static USceneComponent* GetParentComponent(const FAGX_Frame& Frame)
	{
		return Frame.GetParentComponent();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	static FVector GetWorldLocation(const FAGX_Frame& Frame)
	{
		return Frame.GetWorldLocation();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	static FRotator GetWorldRotation(const FAGX_Frame& Frame)
	{
		return Frame.GetWorldRotation();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	void GetWorldLocationAndRotation(
		const FAGX_Frame& Frame, FVector& OutLocation, FRotator& OutRotation)
	{
		Frame.GetWorldLocationAndRotation(OutLocation, OutRotation);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	FVector GetLocationRelativeTo(const FAGX_Frame& Frame, USceneComponent* Component)
	{
		if (Component == nullptr)
		{
			return Frame.GetWorldLocation();
		}
		return Frame.GetLocationRelativeTo(*Component);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	FRotator GetRotationRelativeTo(const FAGX_Frame& Frame, USceneComponent* Component)
	{
		if (Component == nullptr)
		{
			return Frame.GetWorldRotation();
		}
		return Frame.GetRotationRelativeTo(*Component);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Frame")
	void GetRelativeTo(
		const FAGX_Frame& Frame, USceneComponent* Component, FVector& OutLocation,
		FRotator& OutRotation)
	{
		if (Component == nullptr)
		{
			OutLocation = Frame.GetWorldLocation();
			OutRotation = Frame.GetWorldRotation();
			return;
		}
		Frame.GetRelativeTo(*Component, OutLocation, OutRotation);
	}
};
