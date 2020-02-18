#include "AGX_InputTargetSpeed.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/AGX_Constraint1DOF.h"
#include "Constraints/AGX_Constraint2DOF.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

namespace UAGX_InputTargetSpeed_Helpers
{
	bool HasValidParent(UAGX_InputTargetSpeed& Input)
	{
		if (Input.GetOwner() == nullptr)
		{
			return false;
		}
		return Input.GetOwner()->IsA<AAGX_Constraint1DOF>() ||
			   Input.GetOwner()->IsA<AAGX_Constraint2DOF>();
	}
}

UAGX_InputTargetSpeed::UAGX_InputTargetSpeed()
{
	using namespace UAGX_InputTargetSpeed_Helpers;

	PrimaryComponentTick.bCanEverTick = true;
}

void UAGX_InputTargetSpeed::BeginPlay()
{
	using namespace UAGX_InputTargetSpeed_Helpers;

	Super::BeginPlay();

	if (!HasValidParent(*this))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Input Target Speed components should only be created on AGX Constraint 1/2 "
				 "DOF actors. Actor %s isn't one of those."),
			*GetOwner()->GetName());
	}
}

namespace UAGX_InputTargetSpeed_Helpers
{
	using FContext = std::tuple<FAGX_ConstraintTargetSpeedController*, APlayerController*, bool>;
	FContext GetContext(UAGX_InputTargetSpeed& Input)
	{
		FContext Failure {nullptr, nullptr, false};

		APlayerController* Controller = UGameplayStatics::GetPlayerController(&Input, 0);
		if (Controller == nullptr)
		{
			Input.LogErrorMessageOnce(
				TEXT("No player controller available to provide keyboard input."));
			return Failure;
		}

		if (AAGX_Constraint1DOF* Constraint1DOF = Cast<AAGX_Constraint1DOF>(Input.GetOwner()))
		{
			return {&Constraint1DOF->TargetSpeedController, Controller, true};
		}

		if (AAGX_Constraint2DOF* Constraint2DOF = Cast<AAGX_Constraint2DOF>(Input.GetOwner()))
		{
			const bool First = Input.TargetDOF == EAGX_Constraint2DOFFreeDOF::FIRST;
			return {(First ? &Constraint2DOF->TargetSpeedController1
						   : &Constraint2DOF->TargetSpeedController2),
					Controller, true};
		}

		Input.LogErrorMessageOnce(TEXT("Placed below something not a constraint."));
		return Failure;
	}

	void SetControllerState(
		FAGX_ConstraintTargetSpeedController& Controller, float Speed, bool bEnabled)
	{
		if (Controller.Speed == Speed && Controller.bEnable == bEnabled)
		{
			return;
		}

		Controller.Speed = Speed;
		Controller.bEnable = bEnabled;
		Controller.UpdateNativeProperties();
	}
}

void UAGX_InputTargetSpeed::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	using namespace UAGX_InputTargetSpeed_Helpers;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FAGX_ConstraintTargetSpeedController* SpeedController;
	APlayerController* PlayerController;
	bool bIsValid;
	std::tie(SpeedController, PlayerController, bIsValid) = GetContext(*this);
	if (!bIsValid)
	{
		return;
	}

	const bool bForward = PlayerController->IsInputKeyDown(ForwardKey);
	const bool bBackward = PlayerController->IsInputKeyDown(BackwardKey);
	if (bForward && bBackward)
	{
		SetControllerState(*SpeedController, 0.0f, true);
	}
	else if (bForward)
	{
		SetControllerState(*SpeedController, ForwardSpeed, true);
	}
	else if (bBackward)
	{
		SetControllerState(*SpeedController, -BackwardSpeed, true);
	}
	else
	{
		SetControllerState(*SpeedController, 0.0f, !bDisableOnRelease);
	}
}

void UAGX_InputTargetSpeed::LogErrorMessageOnce(const TCHAR* Message)
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
