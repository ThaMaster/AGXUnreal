#include "Constraints/AGX_ConstraintComponentVisualizer.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintDofGraphicsComponent.h"

// Unreal Engine inclues.
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintComponentVisualizer"

namespace
{
	void DrawCoordinateSystemAxes(
		FPrimitiveDrawInterface* PDI, FVector const& AxisLoc, FRotator const& AxisRot, float Scale,
		uint8 DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
	{
		FRotationMatrix R(AxisRot);
		FVector const X = R.GetScaledAxis(EAxis::X);
		FVector const Y = R.GetScaledAxis(EAxis::Y);
		FVector const Z = R.GetScaledAxis(EAxis::Z);

		// UE_LOG(LogAGX, Warning, TEXT("Drawing axes at %.2f, %.2f, %.2f"), AxisLoc.X, AxisLoc.Y,
		// AxisLoc.Z);

		PDI->DrawLine(
			AxisLoc, AxisLoc + X * Scale, FLinearColor::Red, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
		PDI->DrawLine(
			AxisLoc, AxisLoc + Y * Scale, FLinearColor::Green, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
		PDI->DrawLine(
			AxisLoc, AxisLoc + Z * Scale, FLinearColor::Blue, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
	}

	static FVector2D operator*(const FVector& NormalizedCoordinates, const FIntPoint& ViewSize)
	{
		return FVector2D(
			NormalizedCoordinates.X * ViewSize.X, NormalizedCoordinates.Y * ViewSize.Y);
	}

	float GetScreenToWorldFactor(float FOV, float WorldDistance)
	{
		float Hack = 0.5f; // because result seemed a bit off...
		return Hack * 2.0f * WorldDistance * FMath::Atan(FOV / 2.0f);
	}

	float GetWorldSizeFromScreenFactor(float ScreenFactor, float FOV, float WorldDistance)
	{
		return ScreenFactor * GetScreenToWorldFactor(FOV, WorldDistance);
	}

	float GetScreenFactorFromWorldSize(float WorldSize, float FOV, float WorldDistance)
	{
		return WorldSize / GetScreenToWorldFactor(FOV, WorldDistance);
	}

	FBox GetBoundingBox(UAGX_RigidBodyComponent* Body)
	{
		FBox Box(ForceInit);

		const FTransform& BodyToWorld = Body->GetComponentTransform();
		const FTransform WorldToBody = BodyToWorld.Inverse();

		TArray<USceneComponent*> Children;
		Body->GetChildrenComponents(true, Children);
		for (auto Child : Children)
		{
			const FTransform ComponentToBody = Child->GetComponentTransform() * WorldToBody;
			const FBoxSphereBounds BoundInBodySpace = Child->CalcBounds(ComponentToBody);
			Box += BoundInBodySpace.GetBox();
		}
		return Box;
	}

	/// \todo Consider moving GetRigidBody with fallback to FAGX_ConstraintBodyAttachment.
	UAGX_RigidBodyComponent* GetRigidBody(
		const FAGX_RigidBodyReference& BodyReference, AActor* Fallback)
	{
		if (UAGX_RigidBodyComponent* Body = BodyReference.GetRigidBody())
		{
			return Body;
		}

		// Not sure why this is needed when in the Blueprint editor. The intention is that a
		// fallback OwningActor should be stored in the RigidBodyReference.
		if (Fallback == nullptr)
		{
			return nullptr;
		}
		TArray<UAGX_RigidBodyComponent*> AllBodies;
		Fallback->GetComponents(AllBodies, BodyReference.bSearchChildActors);
		for (UAGX_RigidBodyComponent* Candidate : AllBodies)
		{
			if (Candidate->GetFName() == BodyReference.BodyName)
			{
				return Candidate;
			}
		}

		return nullptr;
	}

	void DrawTranslationalPrimitive(
		FPrimitiveDrawInterface* PDI, const FColor& Color, const FTransform& WorldTransform,
		EAxis::Type Axis, float Height, float Offset)
	{
		constexpr int32 NUM_SIDES {64};
		constexpr float CONE_ANGLE {20};

		// DrawWireCone renders the cone along the x-axis with the tip at the origin of
		// WorldTransform. Create a new transform with its x-axis in the negative Axis direction and
		// translate it Height
		// + Offset distance.
		FTransform ConeAlignedTransform;
		const FVector X_w = WorldTransform.GetUnitAxis(EAxis::X);
		const FVector Y_w = WorldTransform.GetUnitAxis(EAxis::Y);
		const FVector Z_w = WorldTransform.GetUnitAxis(EAxis::Z);
		const FVector Origo_w = WorldTransform.GetLocation();

		switch (Axis)
		{
			case EAxis::X:
				ConeAlignedTransform =
					FTransform(-X_w, -Y_w, Z_w, Origo_w + (Height + Offset) * X_w);
				break;
			case EAxis::Y:
				ConeAlignedTransform =
					FTransform(-Y_w, X_w, Z_w, Origo_w + (Height + Offset) * Y_w);
				break;
			case EAxis::Z:
				ConeAlignedTransform =
					FTransform(-Z_w, Y_w, X_w, Origo_w + (Height + Offset) * Z_w);
				break;
		}

		TArray<FVector> Unused;
		DrawWireCone(
			PDI, Unused, ConeAlignedTransform, Height, CONE_ANGLE, NUM_SIDES, Color,
			SDPG_Foreground);
	}

	// Draws NumArrows arrows of height Height along a circle with radius Radius around
	// WorldTransform's x-axis.
	void DrawArrowsAlongCircle(
		FPrimitiveDrawInterface* PDI, const FColor& Color, FTransform WorldTransform, float Radius,
		float Height, int32 NumArrows)
	{
		constexpr int32 NUM_SIDES {32};
		constexpr float CONE_ANGLE {30};

		// LocalArrowTransform is the final location and rotation of the arrow being drawn,
		// expressed in the WorldTransform coordinate system.
		FTransform LocalArrowTransform;
		LocalArrowTransform.SetLocation(FVector(Radius, -Height / 2, 0));
		LocalArrowTransform.SetRotation(FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 90.0f)));

		for (int i = 0; i < NumArrows; i++)
		{
			TArray<FVector> Unused;
			DrawWireCone(
				PDI, Unused, (LocalArrowTransform * WorldTransform), Height, CONE_ANGLE, NUM_SIDES,
				Color, SDPG_Foreground);

			// Rotate WorldTransform by (360 / NumArrows) deg so that the next drawn arrow gets the correct
			// position and orientation.
			WorldTransform.SetRotation(
				WorldTransform.GetRotation() *
				FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 360.0f / NumArrows)));
		}
	}

	void DrawRotationalPrimitive(
		FPrimitiveDrawInterface* PDI, const FColor& Color, const FTransform& WorldTransform,
		EAxis::Type Axis, float Radius, float Offset)
	{
		constexpr int32 NUM_SIDES {64};

		const FVector X_w = WorldTransform.GetUnitAxis(EAxis::X);
		const FVector Y_w = WorldTransform.GetUnitAxis(EAxis::Y);
		const FVector Z_w = WorldTransform.GetUnitAxis(EAxis::Z);
		const FVector Origo_w = WorldTransform.GetLocation();

		FTransform CylinderAlignedTransform;
		const float CylinderHalfHeight = 0.15f * Radius;
		switch (Axis)
		{
			case EAxis::X:
				CylinderAlignedTransform = FTransform(Y_w, Z_w, X_w, Origo_w + Offset * X_w);
				break;
			case EAxis::Y:
				CylinderAlignedTransform = FTransform(Z_w, X_w, Y_w, Origo_w + Offset * Y_w);
				break;
			case EAxis::Z:
				CylinderAlignedTransform = FTransform(X_w, Y_w, Z_w, Origo_w + Offset * Z_w);
				break;
		}

		DrawWireCylinder(
			PDI, CylinderAlignedTransform.GetLocation(),
			CylinderAlignedTransform.GetUnitAxis(EAxis::X),
			CylinderAlignedTransform.GetUnitAxis(EAxis::Y),
			CylinderAlignedTransform.GetUnitAxis(EAxis::Z), Color, Radius, CylinderHalfHeight,
			NUM_SIDES, SDPG_Foreground);

		const float ArrowHeight = 0.6f * Radius;
		DrawArrowsAlongCircle(PDI, Color, CylinderAlignedTransform, Radius, ArrowHeight, 3);
	}

	void RenderBodyMarker(
		const FAGX_ConstraintBodyAttachment& Attachment, UAGX_RigidBodyComponent* Body,
		float CircleScreenFactor, const FColor& Color, const float Thickness,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		// UE_LOG(LogAGX, Log, TEXT("Rendering body markers for body '%s'."), *Body->GetName());

#if 0 // bHighlightUsingBoundingBox
	FBox LocalAABB = GetBoundingBox(Body);

	DrawOrientedWireBox(
		PDI, Body->GetComponentLocation(), Body->GetForwardVector(), Body->GetRightVector(),
		Body->GetUpVector(), LocalAABB.GetExtent(), Color, SDPG_World, Thickness,
		/*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
#endif

		// Highlight Body with circle.
		{
			const FVector Direction =
				(Body->GetComponentLocation() - View->ViewLocation).GetSafeNormal();
			const float Distance = 40.0f;
			const FVector Location = View->ViewLocation + Direction * Distance;
			const float Radius = GetWorldSizeFromScreenFactor(
				CircleScreenFactor, FMath::DegreesToRadians(View->FOV), Distance);

			DrawCircle(
				PDI, Location, View->GetViewRight(), View->GetViewUp(), Color, Radius, 32,
				SDPG_Foreground, Thickness, /*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
		}

		// Draw frame attachment gizmo.
		{
			// It is important to make sure that drawn coordinate system does not interfere
			// with the default transform gizmo. Therefore, make sure thickness of drawn
			// coordinate system is thinner than transform gizmo, so that the transform gizmo is
			// always selectable.
			//
			// If is not thinner, in the scenario where the coordinate system equals the
			// currently active transform gizmo (i.e. visually overlapping), and they are
			// located inside a mesh while the camera is ouside of the mesh, there are
			// difficulties	selecting the transform gizmo (even if HitProxy, depth bias, etc are
			// used).

			/// \todo This doesn't work in the Blueprint editor as RigidBodyReference is currently
			/// implemented RigidBodyReference must be made aware of fallback owners. Can I just set
			/// the OwningActor to the Constraint's owner? What would be cool. What will the
			/// DeatailCustomization do?
			constexpr float FRAME_GIZMO_SCALE {0.2f};

			const float AttachemtFrameDistance =
				FVector::Dist(Attachment.GetGlobalFrameLocation(Body), View->ViewLocation);
			const float FrameGizmoSize = GetWorldSizeFromScreenFactor(
				FRAME_GIZMO_SCALE, FMath::DegreesToRadians(View->FOV), AttachemtFrameDistance);

			DrawCoordinateSystemAxes(
				PDI, Attachment.GetGlobalFrameLocation(Body),
				Attachment.GetGlobalFrameRotation().Rotator(), FrameGizmoSize, SDPG_Foreground,
				0.0f,
				/*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
		}
	}

	void RenderDofPrimitives(
		FPrimitiveDrawInterface* PDI, const FSceneView* View,
		const UAGX_ConstraintComponent* Constraint, const FAGX_ConstraintBodyAttachment& Attachment,
		bool IsViolated)
	{
		constexpr float TRANSLATIONAL_PRIMITIVE_SCALE {0.06f};
		constexpr float ROTATIONAL_PRIMITIVE_SCALE {0.07f};

		const FTransform AttachmentTransform(Attachment.GetGlobalFrameMatrix());
		const float AttachemtFrameDistance =
			FVector::Dist(Attachment.GetGlobalFrameLocation(), View->ViewLocation);

		const float Radius = GetWorldSizeFromScreenFactor(
			ROTATIONAL_PRIMITIVE_SCALE, FMath::DegreesToRadians(View->FOV), AttachemtFrameDistance);
		const float RotOffset = 1.4f * Radius;

		const float Height = GetWorldSizeFromScreenFactor(
			TRANSLATIONAL_PRIMITIVE_SCALE, FMath::DegreesToRadians(View->FOV),
			AttachemtFrameDistance);
		const float TransOffset = 1.4f * Height;

		const FColor RotDofPrimitiveColor = IsViolated ? FColor::Red : FColor(243, 139, 0);
		const FColor TransDofPrimitiveColor = IsViolated ? FColor::Red : FColor(243, 200, 0);
		if (!Constraint->IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_1))
		{
			DrawRotationalPrimitive(
				PDI, RotDofPrimitiveColor, AttachmentTransform, EAxis::X, Radius, RotOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_2))
		{
			DrawRotationalPrimitive(
				PDI, RotDofPrimitiveColor, AttachmentTransform, EAxis::Y, Radius, RotOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_3))
		{
			DrawRotationalPrimitive(
				PDI, RotDofPrimitiveColor, AttachmentTransform, EAxis::Z, Radius, RotOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DOF_FLAG_TRANSLATIONAL_1))
		{
			DrawTranslationalPrimitive(
				PDI, TransDofPrimitiveColor, AttachmentTransform, EAxis::X, Height, TransOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DOF_FLAG_TRANSLATIONAL_2))
		{
			DrawTranslationalPrimitive(
				PDI, TransDofPrimitiveColor, AttachmentTransform, EAxis::Y, Height, TransOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DOF_FLAG_TRANSLATIONAL_3))
		{
			DrawTranslationalPrimitive(
				PDI, TransDofPrimitiveColor, AttachmentTransform, EAxis::Z, Height, TransOffset);
		}
	}
}

void FAGX_ConstraintComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_ConstraintComponent* Constraint = Cast<const UAGX_ConstraintComponent>(Component);
	if (Constraint == nullptr)
		return;

	DrawConstraint(Constraint, View, PDI);

	if (UAGX_ConstraintDofGraphicsComponent* DofGraphics = Constraint->GetDofGraphics1())
	{
		/// Hack to force update of render transform, if for example the constraint uses a
		/// constraint transform actor and it was moved without UAGX_ConstraintDofGraphicsComponent
		/// knowing about it... \todo Might be a better way to do this?
		DofGraphics->OnBecameSelected();
	}
	if (UAGX_ConstraintDofGraphicsComponent* DofGraphics = Constraint->GetDofGraphics2())
	{
		/// Hack to force update of render transform, if for example the constraint uses a
		/// constraint transform actor and it was moved without UAGX_ConstraintDofGraphicsComponent
		/// knowing about it... \todo Might be a better way to do this?
		DofGraphics->OnBecameSelected();
	}
}

void FAGX_ConstraintComponentVisualizer::DrawVisualizationHUD(
	const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	const UAGX_ConstraintComponent* Constraint = Cast<const UAGX_ConstraintComponent>(Component);

	if (!Constraint)
	{
		return;
	}

	DrawConstraintHUD(Constraint, Viewport, View, Canvas);
}

void FAGX_ConstraintComponentVisualizer::DrawConstraint(
	const UAGX_ConstraintComponent* Constraint, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	if (Constraint == nullptr || !Constraint->IsVisible())
	{
		return;
	}

	FString Unused;
	const bool Violated = Constraint->AreFramesInViolatedState(KINDA_SMALL_NUMBER, &Unused);

	const FAGX_RigidBodyReference& BodyReference1 = Constraint->BodyAttachment1.RigidBody;
	const FAGX_RigidBodyReference& BodyReference2 = Constraint->BodyAttachment2.RigidBody;

	AActor* BodyOwnerFallback = Constraint->GetOwner();
	UAGX_RigidBodyComponent* Body1 = GetRigidBody(BodyReference1, BodyOwnerFallback);
	UAGX_RigidBodyComponent* Body2 = GetRigidBody(BodyReference2, BodyOwnerFallback);

	const FColor HighlightColor(243, 139, 0);
	const float HighlightThickness(1.0f);

	if (Body1 != nullptr)
	{
		float CircleScreenFactor = 0.08f;
		RenderBodyMarker(
			Constraint->BodyAttachment1, Body1, CircleScreenFactor, HighlightColor,
			HighlightThickness, View, PDI);
	}
	if (Body2 != nullptr)
	{
		float CircleScreenFactor = 0.05f;
		FColor Color = FColor(
			HighlightColor.R * 0.6f, HighlightColor.G * 0.6f, HighlightColor.B * 0.6f,
			HighlightColor.A);
		RenderBodyMarker(
			Constraint->BodyAttachment2, Body2, CircleScreenFactor, Color, HighlightThickness, View,
			PDI);
	}

	RenderDofPrimitives(PDI, View, Constraint, Constraint->BodyAttachment1, Violated);
	RenderDofPrimitives(PDI, View, Constraint, Constraint->BodyAttachment2, Violated);

	if (Body1 != nullptr && Body2 != nullptr)
	{
		float Distance = 100.0f;

		const FVector DirectionBody1 =
			(Body1->GetComponentLocation() - View->ViewLocation).GetSafeNormal();
		const FVector LocationBody1 = View->ViewLocation + DirectionBody1 * Distance;

		const FVector DirectionAttach1 =
			(Constraint->BodyAttachment1.GetGlobalFrameLocation(Body1) - View->ViewLocation)
				.GetSafeNormal();
		const FVector LocationAttach1 = View->ViewLocation + DirectionAttach1 * Distance;

		const FVector DirectionBody2 =
			(Body2->GetComponentLocation() - View->ViewLocation).GetSafeNormal();
		const FVector LocationBody2 = View->ViewLocation + DirectionBody2 * Distance;

		const FVector DirectionAttach2 =
			(Constraint->BodyAttachment2.GetGlobalFrameLocation(Body2) - View->ViewLocation)
				.GetSafeNormal();
		const FVector LocationAttach2 = View->ViewLocation + DirectionAttach2 * Distance;

		DrawDashedLine(
			PDI, LocationBody1, LocationAttach1, HighlightColor, HighlightThickness,
			SDPG_Foreground,
			/*DepthBias*/ 0.0f);

		DrawDashedLine(
			PDI, LocationBody2, LocationAttach2, HighlightColor, HighlightThickness,
			SDPG_Foreground,
			/*DepthBias*/ 0.0f);

		if (Violated)
		{
			DrawDashedLine(
				PDI, LocationAttach1, LocationAttach2, FColor::Red, HighlightThickness,
				SDPG_Foreground,
				/*DepthBias*/ 0.0f);
		}
	}
}

void FAGX_ConstraintComponentVisualizer::DrawConstraintHUD(
	const UAGX_ConstraintComponent* Constraint, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	if (Constraint == nullptr || !Constraint->IsVisible())
	{
		return;
	}

	FString Message;
	if (Constraint->AreFramesInViolatedState(KINDA_SMALL_NUMBER, &Message))
	{
		FVector2D Position(0.45f, 0.35f);
		FText Text = FText::FromString(
			FString::Printf(TEXT("Constraint Frames In Violated State!\n%s"), *Message));
		UFont* Font = GEngine->GetSubtitleFont();
		FCanvasTextItem CanvasText(
			Position * Canvas->GetViewRect().Size(), Text, Font, FColor::Red);

		Canvas->DrawItem(CanvasText);

		// GEngine->AddOnScreenDebugMessage(45456, 5.f, FColor::Red, TEXT("Constraint Frames In
		// Violated State!"));
	}
}

#undef LOCTEXT_NAMESPACE
