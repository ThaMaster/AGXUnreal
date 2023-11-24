// Copyright 2023, Algoryx Simulation AB.

#include "AGX_Frame.h"

// Unreal Engine includes.
#include "Math/Transform.h"
#include "Components/SceneComponent.h"

FAGX_Frame::FAGX_Frame()
{
	Parent.ComponentType = USceneComponent::StaticClass();
}

void FAGX_Frame::SetParentComponent(USceneComponent* Component)
{
	Parent.OwningActor = Component->GetOwner();
	Parent.Name = Component->GetFName();
}

USceneComponent* FAGX_Frame::GetParentComponent() const
{
	return Parent.GetComponent<USceneComponent>();
}

FVector FAGX_Frame::GetWorldLocation() const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		return LocalLocation;
	}

	return ActualParent->GetComponentTransform().TransformPosition(LocalLocation);
}

FVector FAGX_Frame::GetWorldLocation(const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		ActualParent = &FallbackParent;
	}

	return ActualParent->GetComponentTransform().TransformPosition(LocalLocation);
}

FRotator FAGX_Frame::GetWorldRotation() const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		return LocalRotation;
	}

	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat WorldQuat = ActualParent->GetComponentTransform().TransformRotation(LocalQuat);
	const FRotator WorldRotator = WorldQuat.Rotator();
	return WorldRotator;
}

FRotator FAGX_Frame::GetWorldRotation(const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		ActualParent = &FallbackParent;
	}

	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat WorldQuat = ActualParent->GetComponentTransform().TransformRotation(LocalQuat);
	const FRotator WorldRotator = WorldQuat.Rotator();
	return WorldRotator;
}


void FAGX_Frame::GetWorldLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		OutLocation = LocalLocation;
		OutRotation = LocalRotation;
		return;
	}

	const FTransform Transform = ActualParent->GetComponentTransform();
	OutLocation = Transform.TransformPosition(LocalLocation);
	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat WorldQuat = Transform.TransformRotation(LocalQuat);
	OutRotation = WorldQuat.Rotator();
}

FVector FAGX_Frame::GetLocationRelativeTo(const USceneComponent& Component) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		return Component.GetComponentTransform().InverseTransformPosition(LocalLocation);
	}

	// Construct the transformation that takes us from our Component's coordinate system to the
	// given Component's.
	const FTransform ParentTransform = ActualParent->GetComponentTransform();
	const FTransform TargetTransform = Component.GetComponentTransform();
	const FTransform Transform = ParentTransform.GetRelativeTransform(TargetTransform);
	return Transform.TransformPosition(LocalLocation);
}

FVector FAGX_Frame::GetLocationRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		ActualParent = &FallbackParent;
	}

	// Construct the transformation that takes us from the parent's coordinate system to the
	// given Component's.
	const FTransform ParentTransform = ActualParent->GetComponentTransform();
	const FTransform TargetTransform = Component.GetComponentTransform();
	const FTransform Transform = ParentTransform.GetRelativeTransform(TargetTransform);
	return Transform.TransformPosition(LocalLocation);
}

FRotator FAGX_Frame::GetRotationRelativeTo(const USceneComponent& Component) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		const FQuat LocalQuat = LocalRotation.Quaternion();
		const FQuat RelativeQuat =
			Component.GetComponentTransform().InverseTransformRotation(LocalQuat);
		return RelativeQuat.Rotator();
	}

	const FTransform ParentTransform = ActualParent->GetComponentTransform();
	const FTransform TargetTransform = Component.GetComponentTransform();
	const FTransform Transform = ParentTransform.GetRelativeTransform(TargetTransform);
	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat RelativeQuat = Transform.TransformRotation(LocalQuat);
	return RelativeQuat.Rotator();
}

FRotator FAGX_Frame::GetRotationRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		ActualParent = &FallbackParent;
	}

	const FTransform ParentTransform = ActualParent->GetComponentTransform();
	const FTransform TargetTransform = Component.GetComponentTransform();
	const FTransform Transform = ParentTransform.GetRelativeTransform(TargetTransform);
	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat RelativeQuat = Transform.TransformRotation(LocalQuat);
	return RelativeQuat.Rotator();
}

void FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		const FTransform& Transform = Component.GetComponentTransform();
		OutLocation = Transform.InverseTransformPosition(LocalLocation);
		const FQuat LocalQuat = LocalRotation.Quaternion();
		const FQuat RelativeQuat = Transform.InverseTransformRotation(LocalQuat);
		OutRotation = RelativeQuat.Rotator();
	}

	const FTransform ParentTransform = ActualParent->GetComponentTransform();
	const FTransform TargetTransform = Component.GetComponentTransform();
	const FTransform Transform = ParentTransform.GetRelativeTransform(TargetTransform);
	OutLocation = Transform.TransformPosition(LocalLocation);
	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat RelativeQuat = Transform.TransformRotation(LocalQuat);
	OutRotation = RelativeQuat.Rotator();
}

void FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation,
	const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		ActualParent = &FallbackParent;
	}

	const FTransform ParentTransform = ActualParent->GetComponentTransform();
	const FTransform TargetTransform = Component.GetComponentTransform();
	const FTransform Transform = ParentTransform.GetRelativeTransform(TargetTransform);
	OutLocation = Transform.TransformPosition(LocalLocation);
	const FQuat LocalQuat = LocalRotation.Quaternion();
	const FQuat RelativeQuat = Transform.TransformRotation(LocalQuat);
	OutRotation = RelativeQuat.Rotator();
}
