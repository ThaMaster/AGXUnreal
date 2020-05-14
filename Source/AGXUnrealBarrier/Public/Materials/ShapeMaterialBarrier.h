#pragma once

#include <memory>

#include "Containers/UnrealString.h"

struct FMaterialRef;

/**
 * Acts as an interface to a native AGX Material, and encapsulates it so that it is completely
 * hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FShapeMaterialBarrier
{
public:
	FShapeMaterialBarrier();
	FShapeMaterialBarrier(FShapeMaterialBarrier&& Other) = default;
	FShapeMaterialBarrier(std::unique_ptr<FMaterialRef> Native);
	virtual ~FShapeMaterialBarrier();

	bool HasNative() const;
	FMaterialRef* GetNative();
	const FMaterialRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	void SetName(const FString& Name);
	FString GetName() const;

	void SetDensity(double Density);
	double GetDensity() const;

	void SetYoungsModulus(double YoungsModulus);
	double GetYoungsModulus() const;

	void SetBulkViscosity(double Viscosity);
	double GetBulkViscosity() const;

	void SetDamping(double Damping);
	double GetDamping() const;

	void SetMinMaxElasticRestLength(double MinElasticRestLength, double MaxElasticRestLength);
	double GetMinElasticRestLength() const;
	double GetMaxElasticRestLength() const;

	void SetFrictionEnabled(bool bEnabled);
	bool GetFrictionEnabled() const;

	void SetRoughness(double Roughness);
	double GetRoughness() const;

	void SetSurfaceViscosity(double Viscosity);
	double GetSurfaceViscosity() const;

	void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap);
	double GetAdhesiveForce() const;
	double GetAdhesiveOverlap() const;

private:
	FShapeMaterialBarrier(const FShapeMaterialBarrier&) = delete;
	void operator=(const FShapeMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FMaterialRef> NativeRef;
};
