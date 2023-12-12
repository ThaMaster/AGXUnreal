// Copyright 2023, Algoryx Simulation AB.

#include "PlayRecord/AGX_PlayRecordComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "PlayRecord/AGX_PlayRecord.h"

// Standard library includes.
#include <limits>

UAGX_PlayRecordComponent::UAGX_PlayRecordComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_PlayRecordComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentIndex = 0;
}

namespace AGX_PlayRecordComponent_helpers
{
	void RecordAngle(const UAGX_ConstraintComponent& Constraint, FAGX_PlayRecordState& OutState)
	{
		if (const UAGX_Constraint1DofComponent* Constraint1Dof =
				Cast<const UAGX_Constraint1DofComponent>(&Constraint))
		{
			OutState.Values.Add(Constraint1Dof->GetAngle());
			return;
		}

		if (const UAGX_Constraint2DofComponent* Constraint2Dof =
				Cast<const UAGX_Constraint2DofComponent>(&Constraint))
		{
			OutState.Values.Add(Constraint2Dof->GetAngle(EAGX_Constraint2DOFFreeDOF::FIRST));
			OutState.Values.Add(Constraint2Dof->GetAngle(EAGX_Constraint2DOFFreeDOF::SECOND));
			return;
		}

		UE_LOG(
			LogAGX, Warning, TEXT("RecordAngle was called with unsupported Constraint '%s'."),
			*Constraint.GetName());
	}

	void PrepareForPlayback(UAGX_ConstraintComponent& Constraint)
	{
		static constexpr auto INF = std::numeric_limits<double>::infinity();
		if (UAGX_Constraint1DofComponent* Constraint1Dof =
				Cast<UAGX_Constraint1DofComponent>(&Constraint))
		{
			Constraint1Dof->LockController.SetEnable(true);
			Constraint1Dof->ElectricMotorController.SetEnable(false);
			Constraint1Dof->FrictionController.SetEnable(false);
			Constraint1Dof->TargetSpeedController.SetEnable(false);
			Constraint1Dof->LockController.SetForceRange(FAGX_RealInterval(-INF, INF));
			Constraint1Dof->LockController.SetSpookDamping(0.0333); // Default value.
			Constraint1Dof->LockController.SetCompliance(1e-08); // Default value.
			return;
		}

		if (UAGX_Constraint2DofComponent* Constraint2Dof =
				Cast<UAGX_Constraint2DofComponent>(&Constraint))
		{
			Constraint2Dof->LockController1.SetEnable(true);
			Constraint2Dof->LockController2.SetEnable(true);
			Constraint2Dof->ElectricMotorController1.SetEnable(false);
			Constraint2Dof->ElectricMotorController2.SetEnable(false);
			Constraint2Dof->FrictionController1.SetEnable(false);
			Constraint2Dof->FrictionController2.SetEnable(false);
			Constraint2Dof->TargetSpeedController1.SetEnable(false);
			Constraint2Dof->TargetSpeedController2.SetEnable(false);
			Constraint2Dof->LockController1.SetForceRange(FAGX_RealInterval(-INF, INF));
			Constraint2Dof->LockController2.SetForceRange(FAGX_RealInterval(-INF, INF));
			Constraint2Dof->LockController1.SetSpookDamping(0.0333); // Default value.
			Constraint2Dof->LockController2.SetSpookDamping(0.0333); // Default value.
			Constraint2Dof->LockController1.SetCompliance(1e-08); // Default value.
			Constraint2Dof->LockController2.SetCompliance(1e-08); // Default value.
			return;
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("PrepareForPlayback was called with unsupported Constraint '%s'."),
			*Constraint.GetName());
	}
}

void UAGX_PlayRecordComponent::RecordConstraintPositions(
	const TArray<UAGX_ConstraintComponent*>& Constraints)
{
	if (PlayRecord == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("RecordConstraintPositions was called on '%s' but the given Play Record Asset "
				 "is not set."),
			*GetName());
		return;
	}

	if (CurrentIndex == 0)
	{
		PlayRecord->States.Empty(InitialStatesAllocationSize);
	}

	const auto LastIndex = PlayRecord->States.Add(FAGX_PlayRecordState());
	AGX_CHECK(CurrentIndex == LastIndex);

	FAGX_PlayRecordState& State = PlayRecord->States.Last();
	for (const auto& Constraint : Constraints)
	{
		if (Constraint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Constraint in RecordConstraintPositions. Constraint "
					 "position recording may not give the wanted result."),
				*GetName());
			continue;
		}

		AGX_PlayRecordComponent_helpers::RecordAngle(*Constraint, State);
	}

	CurrentIndex++;
}

void UAGX_PlayRecordComponent::PlayBackConstraintPositions(
	const TArray<UAGX_ConstraintComponent*>& Constraints)
{
	using namespace AGX_PlayRecordComponent_helpers;

	if (PlayRecord == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("PlayBackConstraintPositions was called on '%s' but the given Play Record Asset "
				 "is not set."),
			*GetName());
		return;
	}

	if (PlayRecord->States.Num() == 0)
		return;

	if (CurrentIndex >= PlayRecord->States.Num())
		return; // We have passed the end of the recording.

	if (CurrentIndex == 0)
	{
		// At the beginning of the recording.
		for (auto& Constraint : Constraints)
		{
			if (Constraint != nullptr)
				PrepareForPlayback(*Constraint);
		}
	}

	const int32 NumValuesInState = PlayRecord->States[CurrentIndex].Values.Num();
	auto SetPosition = [NumValuesInState, this](
						   FAGX_ConstraintLockController& LockController,
						   const FString& PlayRecordName, int32 ValueIndex)
	{
		if (ValueIndex >= NumValuesInState)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' was given Play Record with too few entries to control the given "
					 "Constraint, at PlayRecord index: %d."),
				*PlayRecordName, CurrentIndex);
			return;
		}

		LockController.SetPosition(PlayRecord->States[CurrentIndex].Values[ValueIndex]);
	};

	int32 CurrenDofIndex = 0;
	for (UAGX_ConstraintComponent* Constraint : Constraints)
	{
		if (Constraint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Constraint in PlayBackConstraintPositions. Constraint "
					 "position playback may not give the wanted result."),
				*GetName());
			continue;
		}

		if (UAGX_Constraint1DofComponent* Constraint1Dof =
				Cast<UAGX_Constraint1DofComponent>(Constraint))
		{
			SetPosition(Constraint1Dof->LockController, GetName(), CurrenDofIndex);
			CurrenDofIndex++;
		}
		else if (
			UAGX_Constraint2DofComponent* Constraint2Dof =
				Cast<UAGX_Constraint2DofComponent>(Constraint))
		{
			SetPosition(Constraint2Dof->LockController1, GetName(), CurrenDofIndex);
			CurrenDofIndex++;
			SetPosition(Constraint2Dof->LockController2, GetName(), CurrenDofIndex);
			CurrenDofIndex++;
		}
	}

	CurrentIndex++;
}

void UAGX_PlayRecordComponent::Reset()
{
	CurrentIndex = 0;
}
