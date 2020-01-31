#include "AGX_InputTargetSpeed.h"

// AGXUnreal includes.
#include "AGX_Constraint1DOF.h"
#include "AGX_Constraint2DOF.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

namespace UAGX_InputTargetSpeed_Helpers
{
	bool HasValidParent(UAGX_InputTargetSpeed& Input)
	{
		if (Input.GetOwner() != nullptr && Input.GetOwner()->IsA<AAGX_Constraint1DOF>())
		{
			return true;
		}
		if (Input.GetOwner() != nullptr && Input.GetOwner()->IsA<AAGX_Constraint2DOF>())
		{
			return true;
		}
		return false;
	}
}

UAGX_InputTargetSpeed::UAGX_InputTargetSpeed()
{
	using namespace UAGX_InputTargetSpeed_Helpers;

	PrimaryComponentTick.bCanEverTick = true;

	if (!HasValidParent(*this))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Input Target Speed components should only be created on AGX Constraint 1DOF "
				 "actors. Actor %s isn't an AGX Constraint 1DOF actor."),
			*GetOwner()->GetName());
	}
}

void UAGX_InputTargetSpeed::BeginPlay()
{
	using namespace UAGX_InputTargetSpeed_Helpers;

	Super::BeginPlay();

	if (!HasValidParent(*this))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Input Target Speed components should only be created on AGX Constraint 1DOF "
				 "actors. Actor %s isn't an AGX Constraint 1DOF actor."),
			*GetOwner()->GetName());
	}
}

namespace UAGX_InputTargetSpeed_Helpers
{
	using Context = std::tuple<FAGX_ConstraintTargetSpeedController*, APlayerController*, bool>;
	Context GetContext(UAGX_InputTargetSpeed& Input)
	{
		Context Failure {nullptr, nullptr, false};

		APlayerController* Controller = UGameplayStatics::GetPlayerController(&Input, 0);
		if (Controller == nullptr)
		{
			Input.LogOneErrorMessage(
				TEXT("No player controller available to provide keyboard input"));
			return Failure;
		}

		if (AAGX_Constraint1DOF* Constraint1DOF = Cast<AAGX_Constraint1DOF>(Input.GetOwner()))
		{
			return {&Constraint1DOF->TargetSpeedController, Controller, true};
		}

		if (AAGX_Constraint2DOF* Constraint2DOF = Cast<AAGX_Constraint2DOF>(Input.GetOwner()))
		{
			switch (Input.TargetDOF)
			{
				case EConstraintFreeDOF::FIRST:
					return {&Constraint2DOF->TargetSpeedController1, Controller, true};
				case EConstraintFreeDOF::SECOND:
					return {&Constraint2DOF->TargetSpeedController2, Controller, true};
			}
		}

		Input.LogOneErrorMessage(TEXT("Placed below something not a constraint."));
		return Failure;
	}

	using Context1DOF = std::tuple<AAGX_Constraint1DOF*, APlayerController*, bool>;
	using Context2DOF = std::tuple<AAGX_Constraint2DOF*, APlayerController*, bool>;

	template <typename Context>
	Context GetContext(UAGX_InputTargetSpeed& Input)
	{
		using AConstraint =
			typename std::remove_pointer<typename std::tuple_element<0, Context>::type>::type;

		Context Failure {nullptr, nullptr, false};

		AConstraint* Constraint = Cast<AConstraint>(Input.GetOwner());
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

	Context1DOF GetContext1DOF(UAGX_InputTargetSpeed& Input)
	{
		return GetContext<Context1DOF>(Input);
	}

	Context2DOF GetContext2DOF(UAGX_InputTargetSpeed& Input)
	{
		return GetContext<Context2DOF>(Input);
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

		UE_LOG(LogAGX, Log, TEXT("Setting constraint speed %f"), Speed);
	}

	void DoControl(UAGX_InputTargetSpeed& Input)
	{
		FAGX_ConstraintTargetSpeedController* SpeedController;
		APlayerController* PlayerController;
		bool bIsValid;
		std::tie(SpeedController, PlayerController, bIsValid) = GetContext(Input);
		if (!bIsValid)
		{
			return;
		}

		const bool bForward = PlayerController->IsInputKeyDown(Input.ForwardKey);
		const bool bBackward = PlayerController->IsInputKeyDown(Input.BackwardKey);
		if (bForward && bBackward)
		{
			SetControllerState(*SpeedController, 0.0f, true);
		}
		else if (bForward)
		{
			SetControllerState(*SpeedController, Input.ForwardSpeed, true);
		}
		else if (bBackward)
		{
			SetControllerState(*SpeedController, -Input.BackwardSpeed, true);
		}
		else
		{
			SetControllerState(*SpeedController, 0.0f, !Input.bDisableOnRelease);
		}
	}

	void Control1DOF(UAGX_InputTargetSpeed& Input)
	{
		AAGX_Constraint1DOF* Constraint;
		APlayerController* Controller;
		bool bIsValid;
		std::tie(Constraint, Controller, bIsValid) = GetContext1DOF(Input);
		if (!bIsValid)
		{
			return;
		}

		const bool bForward = Controller->IsInputKeyDown(Input.ForwardKey);
		const bool bBackward = Controller->IsInputKeyDown(Input.BackwardKey);
		if (bForward && bBackward)
		{
			SetControllerState(*Constraint, 0.0f, true);
		}
		else if (bForward)
		{
			SetControllerState(*Constraint, Input.ForwardSpeed, true);
		}
		else if (bBackward)
		{
			SetControllerState(*Constraint, -Input.BackwardSpeed, true);
		}
		else
		{
			SetControllerState(*Constraint, 0.0f, !Input.bDisableOnRelease);
		}
	}

	void Control2DOF(UAGX_InputTargetSpeed& Input)
	{
		AAGX_Constraint1DOF* Constraint;
		APlayerController* Controller;
		bool bIsValid;
		std::tie(Constraint, Controller, bIsValid) = GetContext2DOF(Input);
		if (!bIsValid)
		{
			return;
		}
	}
}

void UAGX_InputTargetSpeed::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	using namespace UAGX_InputTargetSpeed_Helpers;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if 1
	DoControl(*this);
#else
	if (GetOwner()->IsA<AAGX_Constraint1DOF>())
	{
		Control1DOF(*this);
	}
	else if (GetOwner()->IsA<AAGX_Constraint2DOF>())
	{
		Control2DOF(*this);
	}
#endif
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
