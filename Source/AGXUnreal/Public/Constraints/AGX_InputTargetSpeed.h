//

#pragma once

// AGXUnreal includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"


// Unreal Engine includes.
#include "InputCoreTypes.h"

#include "AGX_InputTargetSpeed.generated.h"

/**
 * Component that checks for particular keyboard presses and set a constraint
 * motor speed in response.
 *
 * Should only be added to AGX_Constraint1DOF actors.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_InputTargetSpeed : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_InputTargetSpeed();

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		meta = (Tooltip = "Key to trigger forward motion of the constraint."))
	FKey ForwardKey;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		meta = (Tooltip = "The constraint speed forward. cm/s or degrees/s."))
	float ForwardSpeed;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		meta = (Tooltip = "Key to trigger backward motion of the constraint."))
	FKey BackwardKey;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		meta = (Tooltip = "The constraint speed backward. cm/s or degrees/s."))
	float BackwardSpeed;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		meta =
			(Tooltip = "If checked the constraint will move freely when no key is held. If unchecked the constraint will be stopped when no key is held."))
	bool bDisableOnRelease;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Speed Input", meta = (EditCondition="bMustSelectDOF"))
	EAGX_Constraint2DOFFreeDOF TargetDOF;

	// I want to make this a private property, but I don't know how. Setting
	// VisibleInstanceOnly as a workaround/hack for now.
	UPROPERTY(Category = "AGX Constraint Speed Input", VisibleInstanceOnly)
	bool bMustSelectDOF;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void LogErrorMessageOnce(const TCHAR* Message);

private:
	bool bErrorMessageLogged = false;
};
