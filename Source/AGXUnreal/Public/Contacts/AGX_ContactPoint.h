#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ContactPoint.generated.h"

USTRUCT(Category = "AGX", BlueprintType)
struct AGXUNREAL_API FAGX_ContactPoint
{
	GENERATED_BODY()

public:
	FAGX_ContactPoint() = default;
	FAGX_ContactPoint(const FAGX_ContactPoint& InOther);
	FAGX_ContactPoint(FContactPointBarrier&& InBarrier);
	FAGX_ContactPoint& operator=(const FAGX_ContactPoint& InOther);

	bool HasNative() const;

	/**
	 * @return True if this contact point participates in the solve.
	 */
	bool IsEnabled() const;

	/**
	 * @return The depth of the overlap, in cm.
	 */
	float GetDepth() const;

	/**
	 * @return The location of the contact point, in world coordinates.
	 */
	FVector GetLocation() const;

	/**
	 * @return The normal of the contact point, in world coordinates.
	 */
	FVector GetNormal() const;

	/**
	 * @return The U tangent of the contact point, in world coordinates.
	 */
	FVector GetTangentU() const;

	/**
	 * @return The V tangent of the contact point, in world coordinates.
	 */
	FVector GetTangentV() const;

	/**
	 * @return The complete (signed) contact force, including both normal and tangential (friction),
	 * in the world coordinate system.
	 */
	FVector GetForce() const;

	/**
	 * @return The (signed) normal force in world coordinates.
	 */
	FVector GetNormalForce() const;

	/**
	 * @return The (signed) tangential force (friction force) in world coordinates.
	 */
	FVector GetTangentialForce() const;

	/**
	 * @return The contact force in the contact's local coordinate system, ordered Norma, Tangent U,
	 * Tangent V.
	 */
	FVector GetLocalForce() const;

	FVector GetVelocity() const;
	FVector GetWitnessPoint(int32 Index) const;
	float GetArea() const;

private:
	FContactPointBarrier Barrier;
};
