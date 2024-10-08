// Copyright 2024, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AddForceMode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "EditorViewportClient.h"
#include "EditorModeManager.h"

const FEditorModeID FAGX_AddForceMode::EM_AGX_AddForceModeId = TEXT("EM_AGX_AddForceModeId");

void FAGX_AddForceMode::Activate()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
		return;

	if (!World->IsGameWorld())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Add Force Mode can only be activated during play.");
		return;
	}

	if (!GLevelEditorModeTools().IsModeActive(EM_AGX_AddForceModeId))
		GLevelEditorModeTools().ActivateMode(EM_AGX_AddForceModeId);
}

void FAGX_AddForceMode::Deactivate()
{
	if (GLevelEditorModeTools().IsModeActive(EM_AGX_AddForceModeId))
		GLevelEditorModeTools().DeactivateMode(EM_AGX_AddForceModeId);
}

namespace AGX_AddForceMode_helpers
{
	UAGX_RigidBodyComponent* FindRigidBody(const UPrimitiveComponent& Component)
	{
		TArray<USceneComponent*> Ancestors;
		Component.GetParentComponents(Ancestors);

		for (USceneComponent* Ancestor : Ancestors)
		{
			if (auto Rb = Cast<UAGX_RigidBodyComponent>(Ancestor))
				return Rb;
		}

		return nullptr;
	}
}

void FAGX_AddForceMode::OnMouseClickComponent(
	UPrimitiveComponent* Component, const FVector& WorldLocation,
	const FViewportCursorLocation& CursorInfo)
{
	Body.Reset();

	if (Component == nullptr)
		return;

	Body = AGX_AddForceMode_helpers::FindRigidBody(*Component);
	if (Body == nullptr)
		return;

	ForceOriginLocalPos =
		Body->GetComponentTransform().InverseTransformPositionNoScale(WorldLocation);
	ForceOriginInitialDistance = (CursorInfo.GetOrigin() - WorldLocation).Length();

	if (LockConstraint.HasNative())
		DestroyLockConstraint();

	// Create Lock Constraint.
	LockConstraint.AllocateNative(
		*Body->GetNative(), ForceOriginLocalPos, FQuat::Identity, nullptr, WorldLocation,
		FQuat::Identity);
	LockConstraint.SetEnableComputeForces(true);
	LockConstraint.SetCompliance(1.0E-6, 0);
	LockConstraint.SetCompliance(1.0E-6, 1);
	LockConstraint.SetCompliance(1.0E-6, 2);
	LockConstraint.SetCompliance(1.0E-3, 3);
	LockConstraint.SetCompliance(1.0E-3, 4);
	LockConstraint.SetCompliance(1.0E-3, 5);

	auto Sim = UAGX_Simulation::GetFrom(Body.Get());
	Sim->GetNative()->Add(LockConstraint);
}

void FAGX_AddForceMode::OnDeactivateMode()
{
	Deactivate();
}

void FAGX_AddForceMode::OnMouseDrag(const FViewportCursorLocation& CursorInfo)
{
	if (Body == nullptr || !Body->HasNative())
		return;

	const FVector ForceOriginWorld =
		Body->GetComponentTransform().TransformPositionNoScale(ForceOriginLocalPos);

	const FVector ForceEndPointWorld =
		CursorInfo.GetOrigin() + CursorInfo.GetDirection() * ForceOriginInitialDistance;

	DrawDebugLine(
		Body->GetWorld(), ForceOriginWorld, ForceEndPointWorld, FColor::Green, false, -1.f, 99,
		3.f);

	LockConstraint.SetLocalLocation(1, ForceEndPointWorld);

	FVector ConstraintForce, ConstraintTorque;
	LockConstraint.GetLastForce(Body->GetNative(), ConstraintForce, ConstraintTorque);
	Force = ConstraintForce.Length();

	UpdateCursorDecorator();
}

void FAGX_AddForceMode::OnEndMouseDrag()
{
	Body.Reset();
	ForceOriginLocalPos = FVector::ZeroVector;
	ForceOriginInitialDistance = 0.0;
	DestroyCursorDecorator();
	DestroyLockConstraint();
}

FText FAGX_AddForceMode::GetCursorDecoratorText() const
{
	return FText::FromString(FString::Printf(TEXT("%f N"), Force));
}

void FAGX_AddForceMode::UpdateCursorDecorator()
{
	if (!CursorDecoratorWindow.IsValid())
	{
		CursorDecoratorWindow = SWindow::MakeCursorDecorator();
		FSlateApplication::Get().AddWindow(CursorDecoratorWindow.ToSharedRef(), true);
		CursorDecoratorWindow->SetContent(
			SNew(SToolTip).Text(this, &FAGX_AddForceMode::GetCursorDecoratorText));
	}

	CursorDecoratorWindow->MoveWindowTo(
		FSlateApplication::Get().GetCursorPos() + FSlateApplication::Get().GetCursorSize());
}

void FAGX_AddForceMode::DestroyCursorDecorator()
{
	if (CursorDecoratorWindow.IsValid())
	{
		CursorDecoratorWindow->RequestDestroyWindow();
		CursorDecoratorWindow.Reset();
	}
}

void FAGX_AddForceMode::DestroyLockConstraint()
{
	if (!LockConstraint.HasNative())
		return;

	if (auto Sim = UAGX_Simulation::GetFrom(FAGX_EditorUtilities::GetCurrentWorld()))
		Sim->GetNative()->Remove(LockConstraint);

	LockConstraint.ReleaseNative();
}
