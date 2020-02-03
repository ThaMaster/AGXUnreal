#include "Materials/TerrainMaterialBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include <Misc/AssertionMacros.h>

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
	NativeRef->Native->setName(Convert(Name));
}

FString FTerrainMaterialBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

void FTerrainMaterialBarrier::SetAdhesionOverlapFactor(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setAdhesionOverlapFactor(Density);
}

double FTerrainMaterialBarrier::GetAdhesionOverlapFactor() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getAdhesionOverlapFactor();
}

void FTerrainMaterialBarrier::SetCohesion(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setCohesion(Density);
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

void FTerrainMaterialBarrier::SetDilatancyAngle(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setDilatancyAngle(Density);
}

double FTerrainMaterialBarrier::GetDilatancyAngle() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getDilatancyAngle();
}

void FTerrainMaterialBarrier::SetFrictionAngle(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setFrictionAngle(Density);
}

double FTerrainMaterialBarrier::GetFrictionAngle() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getFrictionAngle();
}

void FTerrainMaterialBarrier::SetMaximumDensity(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setMaximumDensity(Density);
}

double FTerrainMaterialBarrier::GetMaximumDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getMaximumDensity();
}

void FTerrainMaterialBarrier::SetPoissonsRatio(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setPoissonsRatio(Density);
}

double FTerrainMaterialBarrier::GetPoissonsRatio() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getPoissonsRatio();
}

void FTerrainMaterialBarrier::SetSwellFactor(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setSwellFactor(Density);
}

double FTerrainMaterialBarrier::GetSwellFactor() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getSwellFactor();
}

void FTerrainMaterialBarrier::SetYoungsModulus(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setYoungsModulus(Density);
}

double FTerrainMaterialBarrier::GetYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getYoungsModulus();
}

void FTerrainMaterialBarrier::SetAngleOfReposeCompactionRate(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setAngleOfReposeCompactionRate(Density);
}

double FTerrainMaterialBarrier::GetAngleOfReposeCompactionRate() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getAngleOfReposeCompactionRate();
}

void FTerrainMaterialBarrier::SetBankStatePhi(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setBankStatePhi(Density);
}

double FTerrainMaterialBarrier::GetBankStatePhi() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getBankStatePhi();
}

void FTerrainMaterialBarrier::SetCompactionTimeRelaxationConstant(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setCompactionTimeRelaxationConstant(Density);
}

double FTerrainMaterialBarrier::GetCompactionTimeRelaxationConstant() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getCompactionTimeRelaxationConstant();
}

void FTerrainMaterialBarrier::SetCompressionIndex(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setCompressionIndex(Density);
}

double FTerrainMaterialBarrier::GetCompressionIndex() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getCompressionIndex();
}

void FTerrainMaterialBarrier::SetHardeningConstantKE(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setHardeningConstantKE(Density);
}

double FTerrainMaterialBarrier::GetHardeningConstantKE() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getHardeningConstantKE();
}

void FTerrainMaterialBarrier::SetHardeningConstantNE(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setHardeningConstantNE(Density);
}

double FTerrainMaterialBarrier::GetHardeningConstantNE() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getHardeningConstantNE();
}

void FTerrainMaterialBarrier::SetPreconsolidationStress(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setPreconsolidationStress(Density);
}

double FTerrainMaterialBarrier::GetPreconsolidationStress() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getPreconsolidationStress();
}

void FTerrainMaterialBarrier::SetStressCutOffFraction(double Density)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setStressCutOffFraction(Density);
}

double FTerrainMaterialBarrier::GetStressCutOffFraction() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getStressCutOffFraction();
}
