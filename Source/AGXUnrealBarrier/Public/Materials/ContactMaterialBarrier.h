#pragma once

#include <memory>

#include "Containers/UnrealString.h"

struct FContactMaterialRef;

/**
 * Acts as an interface to a native AGX Contact Material, and encapsulates it so that it is completely hidden from code
 * that includes this file.
 */
class AGXUNREALBARRIER_API FContactMaterialBarrier
{
public:
	FContactMaterialBarrier();
	virtual ~FContactMaterialBarrier();

	bool HasNative() const;
	FContactMaterialRef* GetNative();
	const FContactMaterialRef* GetNative() const;

	void AllocateNative(const FMaterialBarrier* Material1, const FMaterialBarrier* Material2);
	void ReleaseNative();

	void SetFrictionSolveType(int32 SolveType);
	int32 GetFrictionSolveType() const;

	void SetFrictionModel(int32 FrictionModel);
	int32 GetFrictionModel() const;

	void SetRestitution(double Restitution);
	double GetRestitution() const;

	void SetSurfaceFrictionEnabled(bool bEnabled);
	bool GetSurfaceFrictionEnabled() const;

	void SetFrictionCoefficient(double Coefficient, bool bPrimaryDirection, bool bSecondaryDirection);
	double GetFrictionCoefficient(bool bPrimaryDirection, bool bSecondaryDirection) const;

	void SetSurfaceViscosity(double Viscosity, bool bPrimaryDirection, bool bSecondaryDirection);
	double GetSurfaceViscosity(bool bPrimaryDirection, bool bSecondaryDirection) const;

	void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap);
	double GetAdhesiveForce() const;
	double GetAdhesiveOverlap() const;

	void SetYoungsModulus(double YoungsModulus);
	double GetYoungsModulus() const;

	void SetDamping(double Damping);
	double GetDamping() const;

	void SetMinMaxElasticRestLength(double MinElasticRestLength, double MaxElasticRestLength);
	double GetMinElasticRestLength() const;
	double GetMaxElasticRestLength() const;

	void SetContactReductionMode(int32 ReductionMode);
	int32 GetContactReductionMode() const;

	void SetContactReductionBinResolution(uint8 BinResolution);
	uint8 GetContactReductionBinResolution() const;

	void SetUseContactAreaApproach(bool bUse);
	bool GetUseContactAreaApproach() const;

private:
	FContactMaterialBarrier(const FContactMaterialBarrier&) = delete;
	void operator=(const FContactMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be null.
	std::unique_ptr<FContactMaterialRef> NativeRef;
};
