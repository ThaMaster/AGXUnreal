//

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Speed Input")
	FKey ForwardKey;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Speed Input")
	float ForwardSpeed;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Speed Input")
	FKey BackwardKey;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Speed Input")
	float BackwardSpeed;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Speed Input")
	bool bDisableOnRelease;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void LogOneErrorMessage(const TCHAR* Message);

private:
	bool bErrorMessageLogged = false;
};
