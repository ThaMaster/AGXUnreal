#include "AGX_InputTargetSpeed.h"

// AGXUnreal includes.
#include "AGX_Constraint1DOF.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UAGX_InputTargetSpeed::UAGX_InputTargetSpeed()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAGX_InputTargetSpeed::BeginPlay()
{
	Super::BeginPlay();
}

namespace UAGX_InputTargetSpeed_Helpers
{
	using Context = std::tuple<AAGX_Constraint1DOF*, APlayerController*, bool>;

	Context GetContext(UAGX_InputTargetSpeed& Input)
	{
		Context Failure {nullptr, nullptr, false};

		AAGX_Constraint1DOF* Constraint = Cast<AAGX_Constraint1DOF>(Input.GetOwner());
		if (Constraint == nullptr)
		{
			Input.LogOneErrorMessage(TEXT("Placed below something not a constraint."));
			return Failure;
		}

		APlayerController* Controller = UGameplayStatics::GetPlayerController(&Input, 0);
		if (Controller == nullptr)
		{
			Input.LogOneErrorMessage(
				TEXT("No player controller available to provide keyboard input."));
			return Failure;
		}

		return {Constraint, Controller, true};
	}

	void SetControllerState(AAGX_Constraint1DOF& Constraint, float Speed, bool bEnabled)
	{
		FAGX_ConstraintTargetSpeedController& Controller = Constraint.TargetSpeedController;
		if (Controller.Speed == Speed && Controller.bEnable == bEnabled)
		{
			return;
		}
		Controller.Speed = Speed;
		Controller.bEnable = bEnabled;
		Constraint.UpdateNativeProperties();

		UE_LOG(LogAGX, Log, TEXT("Setting constraint speed %f"), Speed);
	}
}

void UAGX_InputTargetSpeed::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	using namespace UAGX_InputTargetSpeed_Helpers;

	AAGX_Constraint1DOF* Constraint;
	APlayerController* Controller;
	bool bIsValid;
	std::tie(Constraint, Controller, bIsValid) = GetContext(*this);
	if (!bIsValid)
	{
		return;
	}

	const bool bForward = Controller->IsInputKeyDown(ForwardKey);
	const bool bBackward = Controller->IsInputKeyDown(BackwardKey);
	if (bForward && bBackward)
	{
		SetControllerState(*Constraint, 0.0f, true);
	}
	else if (bForward)
	{
		SetControllerState(*Constraint, ForwardSpeed, true);
	}
	else if (bBackward)
	{
		SetControllerState(*Constraint, -BackwardSpeed, true);
	}
	else
	{
		SetControllerState(*Constraint, 0.0f, !bDisableOnRelease);
	}
}

void UAGX_InputTargetSpeed::LogOneErrorMessage(const TCHAR* Message)
{
	if (bErrorMessageLogged)
	{
		return;
	}

	UE_LOG(
		LogAGX, Log, TEXT("UAGX_InputTargetSpeed %s for %s: %s"), *GetName(),
		*GetOwner()->GetName(), Message);
	bErrorMessageLogged = true;
}
