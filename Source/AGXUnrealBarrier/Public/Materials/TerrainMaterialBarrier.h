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
	virtual ~FTerrainMaterialBarrier();

	bool HasNative() const;
	FTerrainMaterialRef* GetNative();
	const FTerrainMaterialRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	void SetName(const FString& Name);
	FString GetName() const;

	// Bulk properties
	void SetAdhesionOverlapFactor(double Density);
	double GetAdhesionOverlapFactor() const;

	void SetCohesion(double Density);
	double GetCohesion() const;

	void SetDensity(double Density);
	double GetDensity() const;

	void SetDilatancyAngle(double Density);
	double GetDilatancyAngle() const;

	void SetFrictionAngle(double Density);
	double GetFrictionAngle() const;

	void SetMaximumDensity(double Density);
	double GetMaximumDensity() const;

	void SetPoissonsRatio(double Density);
	double GetPoissonsRatio() const;

	void SetSwellFactor(double Density);
	double GetSwellFactor() const;

	void SetYoungsModulus(double Density);
	double GetYoungsModulus() const;

	// Compaction properties
	void SetAngleOfReposeCompactionRate(double Density);
	double GetAngleOfReposeCompactionRate() const;

	void SetBankStatePhi(double Density);
	double GetBankStatePhi() const;

	void SetCompactionTimeRelaxationConstant(double Density);
	double GetCompactionTimeRelaxationConstant() const;

	void SetCompressionIndex(double Density);
	double GetCompressionIndex() const;

	void SetHardeningConstantKE(double Density);
	double GetHardeningConstantKE() const;

	void SetHardeningConstantNE(double Density);
	double GetHardeningConstantNE() const;

	void SetPreconsolidationStress(double Density);
	double GetPreconsolidationStress() const;

	void SetStressCutOffFraction(double Density);
	double GetStressCutOffFraction() const;

private:
	FTerrainMaterialBarrier(const FTerrainMaterialBarrier&) = delete;
	void operator=(const FTerrainMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FTerrainMaterialRef> NativeRef;
};
