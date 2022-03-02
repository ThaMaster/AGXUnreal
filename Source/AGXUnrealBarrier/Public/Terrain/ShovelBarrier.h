// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Math/Vector.h"
#include "Math/TwoVectors.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FRigidBodyBarrier;

struct FShovelRef;

/**
 * A Shovel describe the interaction between a rigid body and a terrain.
 *
 * Shovels are the only objects that are able to dynamically deform a terrain
 * and create particles from displaced soil.
 *
 * There is not corresponding UAGX_Shovel class. Instead, shovels are configured
 * by adding a TopEdge, a CuttingEdge, and a CuttingDirection to any Actor with
 * a RigidBody and registering that Actor in the Shovels Array of the
 * AGX_Terrain. ShovelBarriers are created when needed by the AGX_Terrain. There
 * are, however, an FAGX_Shovel class. This is an internal class used only by
 * AGX_Terrain.
 */
class AGXUNREALBARRIER_API FShovelBarrier
{
public:
	FShovelBarrier();
	FShovelBarrier(std::unique_ptr<FShovelRef> Native);
	FShovelBarrier(FShovelBarrier&& Other);
	~FShovelBarrier();

	void SetTopEdge(const FTwoVectors& TopEdge);
	void SetCuttingEdge(const FTwoVectors& CuttingEdge);

	void SetVerticalBladeSoilMergeDistance(double VerticalBladeSoilMergeDistance);
	double GetVerticalBladeSoilMergeDistance() const;

	void SetNoMergeExtensionDistance(double NoMergeExtensionDistance);
	double GetNoMergeExtensionDistance() const;

	void SetPenetrationForceScaling(double PenetrationForceScaling);
	double GetPenetrationForceScaling() const;

	void SetAlwaysRemoveShovelContacts(bool Enable);
	bool GetAlwaysRemoveShovelContacts() const;

	bool HasNative() const;
	void AllocateNative(
		FRigidBodyBarrier& Body, const FTwoVectors& TopEdge, const FTwoVectors& CuttingEdge,
		const FVector& CuttingDirection);
	FShovelRef* GetNative();
	const FShovelRef* GetNative() const;
	void ReleaseNative();

private:
	FShovelBarrier(const FShovelBarrier&) = delete;
	void operator=(const FShovelBarrier&) = delete;

private:
	std::unique_ptr<FShovelRef> NativeRef;
};
