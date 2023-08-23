// Copyright 2023, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_Frame.h"
#include "AgxAutomationCommon.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(FAGX_FrameSpec, "AGXUnreal.Frame", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_FrameSpec)

void FAGX_FrameSpec::Define()
{
	Describe(
		"When no parent",
		[this]()
		{
			It("should return local location unchanged.",
			   [this]()
			   {
				   const FVector LocalLocation(1.0, 2.0, 3.0);
				   FAGX_Frame Frame;
				   Frame.LocalLocation = LocalLocation;
				   TestEqual(
					   TEXT("Location should be LocalLocation"), Frame.GetWorldLocation(),
					   LocalLocation);
			   });

			It("should return local rotation unchanged.",
			   [this]()
			   {
				   const FRotator LocalRotation(10.0, 20.0, 30.0);
				   FAGX_Frame Frame;
				   Frame.LocalRotation = LocalRotation;
				   TestEqual(
					   TEXT("Rotation should be LocalRotation"), Frame.GetWorldRotation(),
					   LocalRotation);
			   });

			It("should return local location and rotation unchanged.",
			   [this]()
			   {
				   const FVector LocalLocation(1.0, 2.0, 3.0);
				   const FRotator LocalRotation(10.0, 20.0, 30.0);
				   FAGX_Frame Frame;
				   Frame.LocalLocation = LocalLocation;
				   Frame.LocalRotation = LocalRotation;
				   FVector WorldLocation;
				   FRotator WorldRotation;
				   Frame.GetWorldLocationAndRotation(WorldLocation, WorldRotation);
				   TestEqual(
					   TEXT("Location should be LocalLocation"), Frame.GetWorldLocation(),
					   LocalLocation);
				   TestEqual(
					   TEXT("Rotation should be LocalRotation"), Frame.GetWorldRotation(),
					   LocalRotation);
			   });
		});

	Describe(
		"When parent at origin",
		[this]()
		{
			It("should return local location unchanged.",
			   [this]()
			   {
				   AActor* Actor = NewObject<AActor>(
					   GetTransientPackage(), TEXT("Test.Frame.ParentAtOrigin.Location.Actor"));
				   USceneComponent* Parent = NewObject<USceneComponent>(
					   Actor, TEXT("Test.Frame.ParentAtOrigin.Location.Parent"));
				   Actor->AddInstanceComponent(Parent);
				   Parent->SetWorldLocation(FVector::ZeroVector);
				   const FVector LocalLocation {1.0, 2.0, 3.0};
				   FAGX_Frame Frame;
				   Frame.SetParentComponent(Parent);
				   Frame.LocalLocation = LocalLocation;
				   TestNotNull(TEXT("Frame should have a parent."), Frame.GetParentComponent());
				   TestEqual(
					   TEXT("Location should be LocalLocation"), Frame.GetWorldLocation(),
					   LocalLocation);
				   Actor->Destroy();
			   });

			It("should return local rotation unchanged.",
			   [this]()
			   {
				   AActor* Actor = NewObject<AActor>(
					   GetTransientPackage(), TEXT("Test.Frame.ParentAtOrigin.Rotatoin.Actor"));
				   USceneComponent* Parent = NewObject<USceneComponent>(
					   Actor, TEXT("Test.Frame.ParentAtOrigin.Rotation.Parent"));
				   Actor->AddInstanceComponent(Parent);
				   Parent->SetWorldRotation(FRotator::ZeroRotator);
				   const FRotator LocalRotation {10.0, 20.0, 30.0};
				   FAGX_Frame Frame;
				   Frame.SetParentComponent(Parent);
				   Frame.LocalRotation = LocalRotation;
				   TestNotNull(TEXT("Frame should have a parent."), Frame.GetParentComponent());
				   TestEqual(
					   TEXT("Rotation should be LocalRotation"), Frame.GetWorldRotation(),
					   LocalRotation);
				   Parent->DestroyComponent();
			   });

			It("should return local location and rotation unchanged.",
			   [this]()
			   {
				   AActor* Actor = NewObject<AActor>(
					   GetTransientPackage(),
					   TEXT("Test.Frame.ParentAtOrigin.LocationRotation.Actor"));
				   USceneComponent* Parent = NewObject<USceneComponent>(
					   Actor, TEXT("Test.Frame.ParentAtOrigin.LocationRotation.Parent"));
				   Actor->AddInstanceComponent(Parent);
				   Parent->SetWorldLocation(FVector::ZeroVector);
				   const FVector LocalLocation(1.0, 2.0, 3.0);
				   const FRotator LocalRotation(10.0, 20.0, 30.0);
				   FAGX_Frame Frame;
				   Frame.SetParentComponent(Parent);
				   Frame.LocalLocation = LocalLocation;
				   Frame.LocalRotation = LocalRotation;
				   FVector WorldLocation;
				   FRotator WorldRotation;
				   Frame.GetWorldLocationAndRotation(WorldLocation, WorldRotation);
				   TestEqual(
					   TEXT("Location should be LocalLocation"), Frame.GetWorldLocation(),
					   LocalLocation);
				   TestEqual(
					   TEXT("Rotation should be LocalRotation"), Frame.GetWorldRotation(),
					   LocalRotation);
			   });
		});

	Describe(
		"When parent is translated",
		[this]()
		{
			It("should return local location translated and rotation unchanged.",
			   [this]()
			   {
				   const FVector ParentLocation {100.0, 200.0, 300.0};
				   AActor* Actor = NewObject<AActor>(
					   GetTransientPackage(), TEXT("Test.Frame.ParentTranslated.Actor"));
				   USceneComponent* Parent = NewObject<USceneComponent>(
					   Actor, TEXT("Test.Frame.ParentTranslated.Parent"));
				   Actor->AddInstanceComponent(Parent);
				   Parent->SetWorldLocation(ParentLocation);
				   const FVector LocalLocation {1.0, 2.0, 3.0};
				   const FRotator LocalRotation {10, 20, 30};
				   FAGX_Frame Frame;
				   Frame.SetParentComponent(Parent);
				   Frame.LocalLocation = LocalLocation;
				   Frame.LocalRotation = LocalRotation;
				   TestEqual(
					   TEXT("Location should be translated."), Frame.GetWorldLocation(),
					   LocalLocation + ParentLocation);
				   TestEqual(
					   TEXT("Rotation should be unchanged."), Frame.GetWorldRotation(),
					   LocalRotation);
				   FVector WorldLocation;
				   FRotator WorldRotation;
				   Frame.GetWorldLocationAndRotation(WorldLocation, WorldRotation);
				   TestEqual(
					   TEXT("Location should be translated."), WorldLocation,
					   LocalLocation + ParentLocation);
				   TestEqual(TEXT("Rotation should be unchanged."), WorldRotation, LocalRotation);
				   Parent->DestroyComponent();
			   });
		});

	Describe(
		"When parent is rotated",
		[this]()
		{
			It("should return location and rotation rotated.",
			   [this]
			   {
				   const FRotator ParentRotation {90.0, 0.0, 0.0};
				   AActor* Actor = NewObject<AActor>(
					   GetTransientPackage(), TEXT("Test.Frame.ParentRotated.Actor"));
				   USceneComponent* Parent =
					   NewObject<USceneComponent>(Actor, TEXT("Test.Frame.ParentRotated.Parent"));
				   Actor->AddInstanceComponent(Parent);
				   Parent->SetWorldRotation(ParentRotation);
				   const FVector LocalLocation {1.0, 2.0, 3.0};
				   const FRotator LocalRotation {10.0, 20.0, 30.0};
				   FAGX_Frame Frame;
				   Frame.SetParentComponent(Parent);
				   Frame.LocalLocation = LocalLocation;
				   Frame.LocalRotation = LocalRotation;
				   // Rotation by 90 degrees pitch (around Y) (right-handed) means:
				   //   X becomes -Z.
				   //   Y is unchanged.
				   //   Z becomes X.
				   // Alternatively:
				   //   X is moved to Z.
				   //   Y is unchanged.
				   //   -Z is moved to X.
				   const FVector ExpectedLocation {
					   -LocalLocation.Z, LocalLocation.Y, LocalLocation.X};
				   TestEqual(
					   TEXT("Location should be rotated."), Frame.GetWorldLocation(),
					   ExpectedLocation);
				   // I found the expected rotations by setting up the same system in the editor
				   // and reading the Details panel.
				   const FRotator ExpectedRotation {67.731224, 117.273140, 145.505524};
				   TestEqual(
					   TEXT("Rotation should be rotated"), Frame.GetWorldRotation(),
					   ExpectedRotation);
				   FVector WorldLocation;
				   FRotator WorldRotation;
				   Frame.GetWorldLocationAndRotation(WorldLocation, WorldRotation);
				   TestEqual(TEXT("Location should be rotated."), WorldLocation, ExpectedLocation);
				   TestEqual(TEXT("Rotation should be identity"), WorldRotation, ExpectedRotation);
				   Parent->DestroyComponent();
			   });
		});

	Describe(
		"When parent is both translated and rotated",
		[this]()
		{
			It("should return both location and rotation translated and rotated.",
				[this]()
				{
					TestEqual(TEXT("TODO: Implement this test."), 1, 2);
				});
		});
}
