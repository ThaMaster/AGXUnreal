// Copyright 2024, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AddForceMode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
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
}

void FAGX_AddForceMode::OnDeactivateMode()
{
	Deactivate();
}

void FAGX_AddForceMode::OnMouseDrag(const FViewportCursorLocation& CursorInfo)
{
	if (Body == nullptr)
		return;

	const FVector ForceOriginWorld =
		Body->GetComponentTransform().TransformPositionNoScale(ForceOriginLocalPos);

	const FVector ForceEndPointWorld =
		CursorInfo.GetOrigin() + CursorInfo.GetDirection() * ForceOriginInitialDistance;

	DrawDebugLine(
		Body->GetWorld(), ForceOriginWorld, ForceEndPointWorld, FColor::Green, false, -1.f, 99,
		3.f);

	// Arbitrarily chosen, scales with the distance so that theresulting force goes as distance^2.
	const double DistanceToForce = (ForceEndPointWorld - ForceOriginWorld).Length() * 10.0;
	const FVector ForceVec = (ForceEndPointWorld - ForceOriginWorld) * DistanceToForce;

	Body->AddForceAtWorldLocation(ForceVec, ForceOriginWorld);
	Force = ForceVec.Length();

	UpdateCursorDecorator();
}

void FAGX_AddForceMode::OnEndMouseDrag()
{
	Body.Reset();
	ForceOriginLocalPos = FVector::ZeroVector;
	ForceOriginInitialDistance = 0.0;
	DestroyCursorDecorator();
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
