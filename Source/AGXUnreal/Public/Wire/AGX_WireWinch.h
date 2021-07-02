#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_LogCategory.h"
#include "Utilities/DoubleInterval.h"
#include "AGX_RigidBodyReference.h"
#include "Wire/WireWinchBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/EngineBaseTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireWinch.generated.h"

/**
 * Holds all the properties that make up the settings of a Wire Winch. The Barrier object and all
 * the functions are in FAGX_WireWinch, which inherits from this struct. We need this extra layer
 * because Unreal Engine require that all structs be copy assignable but our Barrier objects are
 * not copyable. By moving all the members that should be copied by the copy assignment operator
 * to a base class we make it safer to implement the operator.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_WireWinchSettings
{
	GENERATED_BODY()
public:
	/**
	 * The location of the winch. Either in the local coordinate system of the body that this winch
	 * is attached to, or the world coordinate system if the winch isn't attached to any body.
	 *
	 * Only used during setup, cannot be changed once Begin Play has been called.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Winch")
	FVector Location;

	/**
	 * The orientation of the winch within the coordinate system of whatever it is attached to,
	 * either a body or the world. With a zero rotation the winch will point the wire along the X
	 * axis.
	 *
	 * Only used during setup, cannot be changed once Begin Play has been called.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Winch")
	FRotator Rotation;

	// I would perhaps like to make this a BlueprintReadWrite property, but FAGX_RigidBodyReference
	// is currently not a BlueprintType. Should it be? We deliberately did not make it a Blueprint
	// type previously because the type is a bit "special" and we didn't know what the implications
	// of making it a BlueprintType would be. Do we know more now? Should it be made a
	// BlueprintType? What operations on it should we support in Blueprint Visual Scripts?
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FAGX_RigidBodyReference BodyAttachment;

	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	double PulledInLength = 1000.0;

	/**
	 * This mode can be used to facilitate routing. E.g., put all wire in this winch (start of
	 * route), set mode to auto feed, and route. The wire will automatically decrease in this winch
	 * to fulfill the rout without initial tension.
	 *
	 * Only used during setup, cannot be changed once Begin Play has been called.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Winch")
	bool bAutoFeed = false;

	/**
	 * Set to true to enable the motor paying out or hauling in wire. The direction is controlled
	 * by the sign of Target Speed.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	bool bMotorEnabled = true;

	/**
	 * The speed at which this winch is trying to haul in or pay out wire. It may be unable to reach
	 * the target speed because of force range limitations and resistance from the brake.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	double TargetSpeed = -10.0;

	/**
	 * The allowed force range when paying out and hauling in wire. The lower end of the range is
	 * used when hauling in and the upper range when paying out. The lower end must be less than
	 * zero and the upper range must be greater than zero.
	 *
	 * The actual force range on the Native is set to this value while the motor is enabled. When
	 * the motor is disabled the Native's motor force range is instead set to zero.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FAGX_DoubleInterval MotorForceRange = {
		-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};

	/**
	 * Set to true to enable the brake.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	bool bBrakeEnabled = false;

	/**
	 * Set the desired brake force range. The lower end of the range is the force with which this
	 * winch brake can hold the wire from hauling out and the upper end of the range is the force
	 * with which this winch brake can hold the wire from paying out.
	 *
	 * It's important that the lower value is less than zero and the upper larger than zero.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FAGX_DoubleInterval BrakeForceRange = {
		-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};

	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	bool bEmergencyBrakeEnabled = false;
};

/**
 * A Wire Winch is a carrier for the data required to create a Wire Winch along with the Barrier
 * object that houses the native AGX Dynamics instance. It is a simple struct, not any kind of
 * UObject, which means that it can't do much of the work itself, it needs help from an owning
 * Component.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_WireWinch : public FAGX_WireWinchSettings
{
	GENERATED_BODY()

public:
	FAGX_WireWinch() = default;

	/**
	 * Copy constructor that only copies the Properties seen by Unreal, does not copy the Native
	 * Barrier Engine.
	 */
	FAGX_WireWinch(const FAGX_WireWinch& Other);

	/**
	 * Assignment operator that only copies the Properties seen by Unreal, does not copy the Native
	 * Barrier Engine.
	 */
	FAGX_WireWinch& operator=(const FAGX_WireWinch& Other);

	bool SetBodyAttachment(UAGX_RigidBodyComponent* Body);
	UAGX_RigidBodyComponent* GetBodyAttachment() const;

	void SetPulledInLength(double InPulledInLength);
	double GetPulledInLength() const;

	void EnableMotor();
	void DisableMotor();
	void SetMotorEnabled(bool bInEnable);
	bool IsMotorEnabled() const;

	void SetTargetSpeed(double InTargetSpeed);
	double GetTargetSpeed() const;

	void SetMotorForceRange(const FAGX_DoubleInterval& InForceRange);
	void SetMotorForceRange(double InMin, double InMax);
	void SetMotorForceRangeMin(double InMin);
	void SetMotorForceRangeMax(double InMax);

	FAGX_DoubleInterval GetMotorForceRange() const;
	double GetMotorForceRangeMin() const;
	double GetMotorForceRangeMax() const;

	void EnableBrake();
	void DisableBrake();
	void SetBrakeEnabled(bool bEnable);
	bool IsBrakeEnabled() const;

	void SetBrakeForceRange(const FAGX_DoubleInterval& InBrakeForceRange);
	void SetBrakeForceRange(double InMin, double InMax);
	void SetBrakeForceRangeMin(double InMin);
	void SetBrakeForceRangeMax(double InMax);

	FAGX_DoubleInterval GetBrakeForceRange() const;
	double GetBrakeForceRangeMin() const;
	double GetBrakeForceRangeMax() const;

	void EnableEmergencyBrake();
	void DisableEmergencyBrake();
	void SetEmergencyBrakeEnabled(bool bEnable);
	bool IsEmergencyBrakeEnabled() const;

	/**
	 * @return The speed with which the wire is currently being hauled in, for negative speeds, or
	 * payed out, for positive speeds.
	 */
	double GetCurrentSpeed() const;
	double GetCurrentMotorForce() const;
	double GetCurrentBrakeForce() const;

	//~ Begin AGX_NativeOwner interface.
	// We can't do actual inheritance, but we can at least expose the same member functions.
	bool HasNative() const;
	uint64 GetNativeAddress() const;
	void AssignNative(uint64 NativeAddress);
	//~ End AGX_NativeOwner interface.

	void CreateNative();
	FWireWinchBarrier* GetNative();
	const FWireWinchBarrier* GetNative() const;
	FWireWinchBarrier* GetOrCreateNative();
	void WritePropertiesToNative();

public:
	FWireWinchBarrier NativeBarrier;
};

UCLASS()
class AGXUNREAL_API UAGX_WireWinch_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool SetBodyAttachment(UPARAM(ref) FAGX_WireWinch& Winch, UAGX_RigidBodyComponent* Body);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	UAGX_RigidBodyComponent* GetBodyAttachment(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	void SetPulledInLength(UPARAM(ref) FAGX_WireWinch& Winch, float InPulledInLength);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	float GetPulledInLength(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	void SetMotorEnabled(UPARAM(ref) FAGX_WireWinch& Winch, bool bMotorEnabled);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	bool IsMotorEnabled(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	void SetMotorForceRange(UPARAM(ref) FAGX_WireWinch& Winch, float Min, float Max);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	float GetMotorForceRangeMin(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	float GetMotorForceRangeMax(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	void SetBrakeForceRange(UPARAM(ref) FAGX_WireWinch& Winch, float Min, float Max);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	float GetBrakeForceRangeMin(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	float GetBrakeForceRangeMax(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	void SetBrakeEnabled(UPARAM(ref) FAGX_WireWinch& Winch, bool bInBrakeEnabled);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	bool IsBrakeEnabled(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch", Meta = (DisplayName = "SetTargetSpeed"))
	void SetTargetSpeed(UPARAM(ref) FAGX_WireWinch& Winch, float InTargetSpeed);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch", Meta = (DisplayName = "GetTargetSpeed"))
	float GetTargetSpeed(UPARAM(ref) const FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch", Meta = (DisplayName = "GetCurrentSpeed"))
	float GetCurrentSpeed(UPARAM(ref) const FAGX_WireWinch& Winch);
};
