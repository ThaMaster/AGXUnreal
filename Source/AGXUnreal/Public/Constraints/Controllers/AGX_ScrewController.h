#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ScrewController.generated.h"

class FScrewControllerBarrier;

/**
 * Screw controller that puts a relationship between two free DOFs of a
 * constraint, given that one free DOF is translational and the other free DOF
 * is rotational. Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintScrewController : public FAGX_ConstraintController
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The distance, in centimeters along the screw's axis, that is covered by
	 * one complete rotation of the screw (360 degrees).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller", Meta = (EditCondition = "bEnable"))
	double Lead;

	void SetLead(double InLead);
	double GetLead() const;

public:
	FAGX_ConstraintScrewController() = default;
	FAGX_ConstraintScrewController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FScrewControllerBarrier> Barrier);
	void CopyFrom(const FScrewControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintScrenController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Screw Controller")
	static void SetLead(
		UPARAM(ref) FAGX_ConstraintScrewController& Controller, const float Lead)
	{
		Controller.SetLead(static_cast<double>(Lead));
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Lead Controller")
	static float GetLead(UPARAM(ref) FAGX_ConstraintScrewController& Controller)
	{
		return static_cast<float>(Controller.GetLead());
	};

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintScrewController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintScrewController& ControllerRef)
	{
		// ScrewControllers
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
