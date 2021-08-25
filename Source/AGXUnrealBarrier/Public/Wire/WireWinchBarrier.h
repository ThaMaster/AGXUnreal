#pragma once

// AGX Dynamics for Unreal includes.
#include "NativeBarrier.h"
#include "Utilities/DoubleInterval.h" /// @todo Use Unreal Engine double interval once they have it.

class FRigidBodyBarrier;
struct FWireWinchRef;

class AGXUNREALBARRIER_API FWireWinchBarrier : public FNativeBarrier<FWireWinchRef>
{
public:
	using Super = FNativeBarrier<FWireWinchRef>;

	FWireWinchBarrier();
	FWireWinchBarrier(std::unique_ptr<FWireWinchRef> Native);
	FWireWinchBarrier(FWireWinchBarrier&& Other);
	virtual ~FWireWinchBarrier();

	void AllocateNative(
		const FRigidBodyBarrier* Body, const FVector& LocalLocation, const FVector& LocalNormal,
		double PulledInLength);

	/// The body that the winch is attached to. Will be empty when attached to the world.
	FRigidBodyBarrier GetRigidBody() const;

/// @todo Don't know how to read this from the AGX Dynamics API.
#if 0
	/// The position of the winch on the body it's attached to, or in world space if there is no
	/// body.
	FVector GetLocalPosition() const;
#endif

	/// The direction of the winch on the body it's attached to, or in world space if there is no
	/// body.
	FVector GetNormal() const;

	/// Set the length of wire that is held inside the winch. This will create new wire, not move
	/// free wire into the winch.
	void SetPulledInWireLength(double InPulledInLength);

	/// The length of wire that the winch contains currently.
	/// This will decrease during routing/initialization if Auto Feed is enabled.
	double GetPulledInWireLength() const;

	/// Decide if wire should be taken from the winch during routing, or if the routed wire is in
	/// addition to the the initial pulled in length. Only used during initialization.
	void SetAutoFeed(bool bAutoFeed);

	bool GetAutoFeed() const;

	/// Maximum force to push or pull the wire.
	FAGX_DoubleInterval GetForceRange() const;

	/// Set the maximum forces that the winch may use to haul in or pay out wire.
	/// The lower end of the range must be negative or zero and is the maximum force to haul in.
	/// The upper end of the range must be positive or zero and is the maximum force to pay out.
	void SetForceRange(const FAGX_DoubleInterval& InForceRange);

	/// The ability of the winch to slow down the wire when the brake is enabled.
	FAGX_DoubleInterval GetBrakeForceRange() const;

	void SetBrakeForceRange(const FAGX_DoubleInterval& InBrakeForceRange);

	/// Enable or disable forced brake. Will only prevent haul in, does not affect pay out.
	/// Is disabled when speed is set to pay out, i.e, speed > 0.
	void SetEnableForcedBrake(bool bBrakeEnabled);

	/// Whether or not the winch is currently braking.
	bool GetEnableForcedBrake() const;

	/// The speed that the winch tries to haul in or pay out wire with.
	/// Positive values is paying out.
	/// Negative values is hauling in.
	double GetSpeed() const;

	/// Set the speed that the winch tries to haul in or pay out wire with.
	/// Positive values is paying out.
	/// Negative values is hauling in.
	void SetSpeed(double InTargetSpeed);

	/// The current speed of the winch motor.
	/// Positive values is paying out.
	/// Negative values is hauling in.
	double GetCurrentSpeed() const;

	/// \return The force that the motor is currently applying.
	double GetCurrentForce() const;

	/// \return The force that the brake is currently applying.
	double GetCurrentBrakeForce() const;

	bool HasWire() const;

	FGuid GetGuid() const;

protected:
/// @todo Determine if we need to override these.
#if 0
	virtual void PreNativeChanged() override;
	virtual void PostNativeChanged() override;
#endif
};
