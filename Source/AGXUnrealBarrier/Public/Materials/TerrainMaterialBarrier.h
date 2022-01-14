// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include <memory>

#include "Containers/UnrealString.h"

struct FTerrainMaterialRef;

/**
 * Acts as an interface to a native AGX Terrain Material, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FTerrainMaterialBarrier
{
public:
	FTerrainMaterialBarrier();
	FTerrainMaterialBarrier(FTerrainMaterialBarrier&& Other);
	FTerrainMaterialBarrier(std::unique_ptr<FTerrainMaterialRef> Native);
	virtual ~FTerrainMaterialBarrier();

	bool HasNative() const;
	FTerrainMaterialRef* GetNative();
	const FTerrainMaterialRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	void SetName(const FString& Name);
	FString GetName() const;

	// Bulk properties
	void SetAdhesionOverlapFactor(double AdhesionOverlapFactor);
	double GetAdhesionOverlapFactor() const;

	void SetCohesion(double Cohesion);
	double GetCohesion() const;

	void SetDensity(double Density);
	double GetDensity() const;

	void SetDilatancyAngle(double DilatancyAngle);
	double GetDilatancyAngle() const;

	void SetFrictionAngle(double FrictionAngle);
	double GetFrictionAngle() const;

	void SetMaximumDensity(double MaxDensity);
	double GetMaximumDensity() const;

	void SetPoissonsRatio(double PoissonsRatio);
	double GetPoissonsRatio() const;

	void SetSwellFactor(double SwellFactor);
	double GetSwellFactor() const;

	void SetYoungsModulus(double YoungsModulus);
	double GetYoungsModulus() const;

	// Compaction properties
	void SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate);
	double GetAngleOfReposeCompactionRate() const;

	void SetBankStatePhi(double Phi0);
	double GetBankStatePhi() const;

	void SetCompactionTimeRelaxationConstant(double CompactionTimeRelaxationConstant);
	double GetCompactionTimeRelaxationConstant() const;

	void SetCompressionIndex(double CompressionIndex);
	double GetCompressionIndex() const;

	void SetHardeningConstantKE(double K_e);
	double GetHardeningConstantKE() const;

	void SetHardeningConstantNE(double N_e);
	double GetHardeningConstantNE() const;

	void SetPreconsolidationStress(double PreconsolidationStress);
	double GetPreconsolidationStress() const;

	void SetStressCutOffFraction(double StressCutOffFraction);
	double GetStressCutOffFraction() const;

private:
	FTerrainMaterialBarrier(const FTerrainMaterialBarrier&) = delete;
	void operator=(const FTerrainMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FTerrainMaterialRef> NativeRef;
};
