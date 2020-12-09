#include "Materials/TerrainMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/version.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"

FTerrainMaterialBarrier::FTerrainMaterialBarrier()
	: NativeRef {new FTerrainMaterialRef}
{
}

FTerrainMaterialBarrier::~FTerrainMaterialBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainMaterialRef.
}

bool FTerrainMaterialBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTerrainMaterialRef* FTerrainMaterialBarrier::GetNative()
{
	return NativeRef.get();
}

const FTerrainMaterialRef* FTerrainMaterialBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTerrainMaterialBarrier::AllocateNative(const FString& Name)
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::TerrainMaterial(Convert(Name));
}

void FTerrainMaterialBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FTerrainMaterialBarrier::SetName(const FString& Name)
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	NativeRef->Native->setDescription(Convert(Name));
#else
	NativeRef->Native->setName(Convert(Name));
#endif
}

FString FTerrainMaterialBarrier::GetName() const
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	return Convert(NativeRef->Native->getDescription());
#else
	return Convert(NativeRef->Native->getName());
#endif
}

void FTerrainMaterialBarrier::SetAdhesionOverlapFactor(double AdhesionOverlapFactor)
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	NativeRef->Native->getParticleProperties()->setAdhesionOverlapFactor(AdhesionOverlapFactor);
#else
	NativeRef->Native->getBulkProperties()->setAdhesionOverlapFactor(AdhesionOverlapFactor);
#endif
}

double FTerrainMaterialBarrier::GetAdhesionOverlapFactor() const
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	return NativeRef->Native->getParticleProperties()->getAdhesionOverlapFactor();
#else
	return NativeRef->Native->getBulkProperties()->getAdhesionOverlapFactor();
#endif
}

void FTerrainMaterialBarrier::SetCohesion(double Cohesion)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setCohesion(Cohesion);
}

double FTerrainMaterialBarrier::GetCohesion() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getCohesion();
}

void FTerrainMaterialBarrier::SetDensity(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setDensity(Density);
}

double FTerrainMaterialBarrier::GetDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getDensity();
}

void FTerrainMaterialBarrier::SetDilatancyAngle(double DilatancyAngle)
{
	check(HasNative());
	double DilatancyAngleRad = FMath::DegreesToRadians(DilatancyAngle);
	NativeRef->Native->getBulkProperties()->setDilatancyAngle(DilatancyAngleRad);
}

double FTerrainMaterialBarrier::GetDilatancyAngle() const
{
	check(HasNative());
	double DilatancyAngleRad = NativeRef->Native->getBulkProperties()->getDilatancyAngle();
	return FMath::RadiansToDegrees(DilatancyAngleRad);
}

void FTerrainMaterialBarrier::SetFrictionAngle(double FrictionAngle)
{
	check(HasNative());
	double FricAngRad = FMath::DegreesToRadians(FrictionAngle);
	NativeRef->Native->getBulkProperties()->setFrictionAngle(FricAngRad);
}

double FTerrainMaterialBarrier::GetFrictionAngle() const
{
	check(HasNative());
	double FricAngRad = NativeRef->Native->getBulkProperties()->getDilatancyAngle();
	return FMath::RadiansToDegrees(FricAngRad);
}

void FTerrainMaterialBarrier::SetMaximumDensity(double MaxDensity)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setMaximumDensity(MaxDensity);
}

double FTerrainMaterialBarrier::GetMaximumDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getMaximumDensity();
}

void FTerrainMaterialBarrier::SetPoissonsRatio(double PoissonsRatio)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setPoissonsRatio(PoissonsRatio);
}

double FTerrainMaterialBarrier::GetPoissonsRatio() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getPoissonsRatio();
}

void FTerrainMaterialBarrier::SetSwellFactor(double SwellFactor)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setSwellFactor(SwellFactor);
}

double FTerrainMaterialBarrier::GetSwellFactor() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getSwellFactor();
}

void FTerrainMaterialBarrier::SetYoungsModulus(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setYoungsModulus(YoungsModulus);
}

double FTerrainMaterialBarrier::GetYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getYoungsModulus();
}

void FTerrainMaterialBarrier::SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setAngleOfReposeCompactionRate(AngleOfReposeCompactionRate);
}

double FTerrainMaterialBarrier::GetAngleOfReposeCompactionRate() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getAngleOfReposeCompactionRate();
}

void FTerrainMaterialBarrier::SetBankStatePhi(double Phi0)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setBankStatePhi(Phi0);
}

double FTerrainMaterialBarrier::GetBankStatePhi() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getBankStatePhi();
}

void FTerrainMaterialBarrier::SetCompactionTimeRelaxationConstant(double CompactionTimeRelaxationConstant)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setCompactionTimeRelaxationConstant(CompactionTimeRelaxationConstant);
}

double FTerrainMaterialBarrier::GetCompactionTimeRelaxationConstant() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getCompactionTimeRelaxationConstant();
}

void FTerrainMaterialBarrier::SetCompressionIndex(double CompressionIndex)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setCompressionIndex(CompressionIndex);
}

double FTerrainMaterialBarrier::GetCompressionIndex() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getCompressionIndex();
}

void FTerrainMaterialBarrier::SetHardeningConstantKE(double K_e)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setHardeningConstantKE(K_e);
}

double FTerrainMaterialBarrier::GetHardeningConstantKE() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getHardeningConstantKE();
}

void FTerrainMaterialBarrier::SetHardeningConstantNE(double N_e)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setHardeningConstantNE(N_e);
}

double FTerrainMaterialBarrier::GetHardeningConstantNE() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getHardeningConstantNE();
}

void FTerrainMaterialBarrier::SetPreconsolidationStress(double PreconsolidationStress)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setPreconsolidationStress(PreconsolidationStress);
}

double FTerrainMaterialBarrier::GetPreconsolidationStress() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getPreconsolidationStress();
}

void FTerrainMaterialBarrier::SetStressCutOffFraction(double StressCutOffFraction)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setStressCutOffFraction(StressCutOffFraction);
}

double FTerrainMaterialBarrier::GetStressCutOffFraction() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getStressCutOffFraction();
}
