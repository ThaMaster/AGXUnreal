// Copyright 2023, Algoryx Simulation AB.

#include "PlayRecord/AGX_PlayRecordComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
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

void UAGX_PlayRecordComponent::RecordConstraintPositions(
	const TArray<UAGX_Constraint1DofComponent*>& Constraints)
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
	State.Values.Reserve(Constraints.Num());

	for (const auto& Constraint : Constraints)
	{
		if (Constraint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Constraint in RecordConstraintPositions. Constraint "
					 "position recording may not work as expected."),
				*GetName());
			continue;
		}

		State.Values.Add(Constraint->GetAngle());
	}

	CurrentIndex++;
}

void UAGX_PlayRecordComponent::PlayBackConstraintPositions(
	const TArray<UAGX_Constraint1DofComponent*>& Constraints)
{
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
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("PlayBackConstraintPositions was called on '%s' but the given Play Record Asset "
				 "does not contain any data."),
			*GetName());
		return;
	}

	if (CurrentIndex >= PlayRecord->States.Num())
		return; // We have passed the end of the recording.

	const int32 NumConstraintsInState = PlayRecord->States[CurrentIndex].Values.Num();
	if (Constraints.Num() != NumConstraintsInState)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("PlayBackConstraintPositions was called on '%s' but the number of Constraints "
				 "given does not match the number of Constraints in the Play Record Asset. Got %d "
				 "Constraints but %d was found in the Play Record Asset at index %d."),
			*GetName(), Constraints.Num(), NumConstraintsInState, CurrentIndex);
		return;
	}

	if (CurrentIndex == 0)
	{
		// At the beginning of the recording; setup secondary constraints for playback.
		for (auto& Constraint : Constraints)
		{
			if (Constraint == nullptr)
			{
				continue;
			}

			if (!Constraint->LockController.GetEnable())
				Constraint->LockController.SetEnable(true);

			if (Constraint->TargetSpeedController.GetEnable())
				Constraint->TargetSpeedController.SetEnable(false);

			static constexpr auto INF = std::numeric_limits<double>::infinity();
			Constraint->LockController.SetForceRange(FAGX_RealInterval(-INF, INF));
		}
	}

	for (int32 i = 0; i < NumConstraintsInState; i++)
	{
		if (Constraints[i] == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Constraint in PlayBackConstraintPositions. Constraint "
					 "position playback may not work as expected."),
				*GetName());
			continue;
		}

		Constraints[i]->LockController.SetPosition(PlayRecord->States[CurrentIndex].Values[i]);
	}

	CurrentIndex++;
}
