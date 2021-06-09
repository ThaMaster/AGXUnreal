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
	~FWireWinchBarrier();

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
	FVector GetLocalNormal() const;


	void SetPulledInLength(double InPulledInLength);

	/// The length of wire that the winch contains currently.
	/// This will decrease during routing/initialization if Auto Feed is enabled.
	double GetPulledInLength() const;

	void SetAutoFeed(bool bAutoFeed);

	bool GetAutoFeed() const;

	/// Maximum force to push or pull the wire.
	FAGX_DoubleInterval GetForceRange() const;

	void SetForceRange(const FAGX_DoubleInterval& InForceRange);

	/// The ability of the winch to slow down the wire when the brake is enabled.
	FAGX_DoubleInterval GetBrakeForceRange() const;

	void SetBrakeForceRange(const FAGX_DoubleInterval& InBrakeForceRange);

	void SetBrakeEnabled(bool bBrakeEnabled);

	/// Whether or not the winch is currently braking.
	bool IsBrakeEnabled() const;

	/// The speed that wire is being pulled in or payed out with.
	double GetTargetSpeed() const;

	void SetTargetSpeed(double InTargetSpeed);

	/// The current speed of the winch motor.
	double GetCurrentSpeed() const;

	/// Whether route nodes should take wire from the winch,
	//		or create new wire.
	bool IsAutoFeed() const;

	FGuid GetGuid() const;

protected:
/// @todo Determine if we need to override these
#if 0
	virtual void PreNativeChanged() override;
	virtual void PostNativeChanged() override;
#endif
};
