#include "Materials/AGX_TerrainMaterialBase.h"

// AGX Dynamics for Unreal includes.
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

// Bulk properties.
void UAGX_TerrainMaterialBase::SetAdhesionOverlapFactor(float AdhesionOverlapFactor)
{
	TerrainBulk.AdhesionOverlapFactor = static_cast<double>(AdhesionOverlapFactor);
}

float UAGX_TerrainMaterialBase::GetAdhesionOverlapFactor() const
{
	return static_cast<float>(TerrainBulk.AdhesionOverlapFactor);
}

void UAGX_TerrainMaterialBase::SetCohesion(float Cohesion)
{
	TerrainBulk.Cohesion = static_cast<double>(Cohesion);
}

float UAGX_TerrainMaterialBase::GetCohesion() const
{
	return static_cast<float>(TerrainBulk.Cohesion);
}

void UAGX_TerrainMaterialBase::SetDensity(float Density)
{
	TerrainBulk.Density = static_cast<double>(Density);
}

float UAGX_TerrainMaterialBase::GetDensity() const
{
	return static_cast<float>(TerrainBulk.Density);
}

void UAGX_TerrainMaterialBase::SetDilatancyAngle(float DilatancyAngle)
{
	TerrainBulk.DilatancyAngle = static_cast<double>(DilatancyAngle);
}

float UAGX_TerrainMaterialBase::GetDilatancyAngle() const
{
	return static_cast<float>(TerrainBulk.DilatancyAngle);
}

void UAGX_TerrainMaterialBase::SetFrictionAngle(float FrictionAngle)
{
	TerrainBulk.FrictionAngle = static_cast<double>(FrictionAngle);
}

float UAGX_TerrainMaterialBase::GetFrictionAngle() const
{
	return static_cast<float>(TerrainBulk.FrictionAngle);
}

void UAGX_TerrainMaterialBase::SetMaxDensity(float MaxDensity)
{
	TerrainBulk.MaxDensity = static_cast<double>(MaxDensity);
}

float UAGX_TerrainMaterialBase::GetMaxDensity() const
{
	return static_cast<float>(TerrainBulk.MaxDensity);
}

void UAGX_TerrainMaterialBase::SetPoissonsRatio(float PoissonsRatio)
{
	TerrainBulk.PoissonsRatio = static_cast<double>(PoissonsRatio);
}

float UAGX_TerrainMaterialBase::GetPoissonsRatio() const
{
	return static_cast<float>(TerrainBulk.PoissonsRatio);
}

void UAGX_TerrainMaterialBase::SetSwellFactor(float SwellFactor)
{
	TerrainBulk.SwellFactor = static_cast<double>(SwellFactor);
}

float UAGX_TerrainMaterialBase::GetSwellFactor() const
{
	return static_cast<float>(TerrainBulk.SwellFactor);
}

void UAGX_TerrainMaterialBase::SetYoungsModulus(float YoungsModulus)
{
	TerrainBulk.YoungsModulus = static_cast<double>(YoungsModulus);
}

float UAGX_TerrainMaterialBase::GetYoungsModulus() const
{
	return static_cast<float>(TerrainBulk.YoungsModulus);
}

// Compaction properties.
void UAGX_TerrainMaterialBase::SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate)
{
	TerrainCompaction.AngleOfReposeCompactionRate = static_cast<double>(AngleOfReposeCompactionRate);
}

float UAGX_TerrainMaterialBase::GetAngleOfReposeCompactionRate() const
{
	return static_cast<float>(TerrainCompaction.AngleOfReposeCompactionRate);
}

void UAGX_TerrainMaterialBase::SetBankStatePhi(float Phi0)
{
	TerrainCompaction.Phi0 =
		static_cast<double>(Phi0);
}

float UAGX_TerrainMaterialBase::GetBankStatePhi() const
{
	return static_cast<float>(TerrainCompaction.Phi0);
}

void UAGX_TerrainMaterialBase::SetCompactionTimeRelaxationConstant(float CompactionTimeRelaxationConstant)
{
	TerrainCompaction.CompactionTimeRelaxationConstant =
		static_cast<double>(CompactionTimeRelaxationConstant);
}

float UAGX_TerrainMaterialBase::GetCompactionTimeRelaxationConstant() const
{
	return static_cast<float>(TerrainCompaction.CompactionTimeRelaxationConstant);
}

void UAGX_TerrainMaterialBase::SetCompressionIndex(float CompressionIndex)
{
	TerrainCompaction.CompressionIndex =
		static_cast<double>(CompressionIndex);
}

float UAGX_TerrainMaterialBase::GetCompressionIndex() const
{
	return static_cast<float>(TerrainCompaction.CompressionIndex);
}

void UAGX_TerrainMaterialBase::SetHardeningConstantKe(float K_e)
{
	TerrainCompaction.K_e =
		static_cast<double>(K_e);
}

float UAGX_TerrainMaterialBase::GetHardeningConstantKe() const
{
	return static_cast<float>(TerrainCompaction.K_e);
}

void UAGX_TerrainMaterialBase::SetHardeningConstantNe(float N_e)
{
	TerrainCompaction.N_e =
		static_cast<double>(N_e);
}

float UAGX_TerrainMaterialBase::GetHardeningConstantNe() const
{
	return static_cast<float>(TerrainCompaction.N_e);
}

void UAGX_TerrainMaterialBase::SetPreconsolidationStress(float PreconsolidationStress)
{
	TerrainCompaction.PreconsolidationStress =
		static_cast<double>(PreconsolidationStress);
}

float UAGX_TerrainMaterialBase::GetPreconsolidationStress() const
{
	return static_cast<float>(TerrainCompaction.PreconsolidationStress);
}

void UAGX_TerrainMaterialBase::SetStressCutOffFraction(float StressCutOffFraction)
{
	TerrainCompaction.StressCutOffFraction =
		static_cast<double>(StressCutOffFraction);
}

float UAGX_TerrainMaterialBase::GetStressCutOffFraction() const
{
	return static_cast<float>(TerrainCompaction.StressCutOffFraction);
}

void UAGX_TerrainMaterialBase::CopyFrom(const FTerrainMaterialBarrier& Source)
{
	TerrainBulk = FAGX_TerrainBulkProperties();
	TerrainBulk.AdhesionOverlapFactor = Source.GetAdhesionOverlapFactor();
	TerrainBulk.Cohesion = Source.GetCohesion();
	TerrainBulk.Density = Source.GetDensity();
	TerrainBulk.DilatancyAngle = Source.GetDilatancyAngle();
	TerrainBulk.FrictionAngle = Source.GetFrictionAngle();
	TerrainBulk.MaxDensity = Source.GetMaximumDensity();
	TerrainBulk.PoissonsRatio = Source.GetPoissonsRatio();
	TerrainBulk.SwellFactor = Source.GetSwellFactor();
	TerrainBulk.YoungsModulus = Source.GetYoungsModulus();

	TerrainCompaction = FAGX_TerrainCompactionProperties();
	TerrainCompaction.AngleOfReposeCompactionRate = Source.GetAngleOfReposeCompactionRate();
	TerrainCompaction.Phi0 = Source.GetBankStatePhi();
	TerrainCompaction.CompactionTimeRelaxationConstant = Source.GetCompactionTimeRelaxationConstant();
	TerrainCompaction.CompressionIndex = Source.GetCompressionIndex();
	TerrainCompaction.K_e = Source.GetHardeningConstantKE();
	TerrainCompaction.N_e = Source.GetHardeningConstantNE();
	TerrainCompaction.PreconsolidationStress = Source.GetPreconsolidationStress();
	TerrainCompaction.StressCutOffFraction = Source.GetStressCutOffFraction();
}

void UAGX_TerrainMaterialBase::CopyTerrainMaterialProperties(const UAGX_TerrainMaterialBase* Source)
{
	if (Source)
	{
		// As of now, this property is not used for terrain (replaced by the terrain specific bulk
		// properties) and will always have default values.
		Bulk = Source->Bulk;

		Surface = Source->Surface;
		TerrainBulk = Source->TerrainBulk;
		TerrainCompaction = Source->TerrainCompaction;
	}
}
