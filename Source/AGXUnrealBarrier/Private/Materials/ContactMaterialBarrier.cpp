// Copyright 2022, Algoryx Simulation AB.


#include "Materials/ContactMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "AGXBarrierFactories.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "TypeConversions.h"

#include <Misc/AssertionMacros.h>

namespace
{
	/// \note Doing conversion with integers instead of the Unreal enums to avoid having to define
	/// the enums in the AGXUnrealBarrier module, and therefore also having to include
	/// AGXUnrealBarrier files in AGXUnreal headers, which is good to avoid for future minimal
	/// distribution to clients.

	agx::FrictionModelRef ConvertFrictionModelToAgx(int32 FrictionModelType)
	{
		// Input value refers to EAGX_FrictionModel in AGX_ContactMaterialEnums.h
		switch (FrictionModelType)
		{
			case 0:
				return nullptr; // "not defined"
			case 1:
				return new agx::BoxFrictionModel();
			case 2:
				return new agx::ScaleBoxFrictionModel();
			case 3:
				return new agx::IterativeProjectedConeFriction();
			default:
				check(!"ConvertFrictionModelToAgx received unsupported value");
				return nullptr;
		}
	}

	int32 ConvertFrictionModelToUnreal(const agx::FrictionModel* FrictionModel)
	{
		// Output value refers to EAGX_FrictionModel in AGX_ContactMaterialEnums.h

		if (dynamic_cast<const agx::BoxFrictionModel*>(FrictionModel))
		{
			return 1;
		}
		else if (dynamic_cast<const agx::IterativeProjectedConeFriction*>(FrictionModel))
		{
			// Since IterativeProjectedConeFriction inherits from ScaleBoxFrictionModel, we must try
			// casting to IterativeProjectedConeFriction before ScaleBoxFrictionModel.
			return 3;
		}
		else if (dynamic_cast<const agx::ScaleBoxFrictionModel*>(FrictionModel))
		{
			return 2;
		}
		else
		{
			return 0; // Zero for "not defined"
		}
	}

	agx::FrictionModel::SolveType ConvertSolveTypeToAgx(int32 SolveType)
	{
		// Input refers to EAGX_ContactSolver in AGX_ContactMaterialEnums.h
		return static_cast<agx::FrictionModel::SolveType>(SolveType);
	}

	int32 ConvertSolveTypeToUnreal(agx::FrictionModel::SolveType SolveType)
	{
		// Output refers to EAGX_ContactSolver in AGX_ContactMaterialEnums.h
		return static_cast<int32>(SolveType);
	}

	agx::ContactMaterial::ContactReductionMode ConvertReductionModeToAgx(int32 ReductionMode)
	{
		// Input refers to EAGX_ContactReductionMode in AGX_ContactMaterialEnums.h
		return static_cast<agx::ContactMaterial::ContactReductionMode>(ReductionMode);
	}

	int32 ConvertReductionModeToUnreal(agx::ContactMaterial::ContactReductionMode ReductionMode)
	{
		// Output refers to EAGX_ContactReductionMode in AGX_ContactMaterialEnums.h
		return static_cast<int32>(ReductionMode);
	}

	agx::ContactMaterial::FrictionDirection ConvertDirectionToAgx(
		bool bPrimaryDirection, bool bSecondaryDirection)
	{
		if (bPrimaryDirection && bSecondaryDirection)
		{
			return agx::ContactMaterial::BOTH_PRIMARY_AND_SECONDARY;
		}
		else if (bPrimaryDirection)
		{
			return agx::ContactMaterial::PRIMARY_DIRECTION;
		}
		else if (bSecondaryDirection)
		{
			return agx::ContactMaterial::SECONDARY_DIRECTION;
		}
		else
		{
			check(!"ConvertFrictionDirectionToAgx received unsupported value");
			return agx::ContactMaterial::BOTH_PRIMARY_AND_SECONDARY;
		}
	}
}

FContactMaterialBarrier::FContactMaterialBarrier()
	: NativeRef {new FContactMaterialRef}
{
}

FContactMaterialBarrier::FContactMaterialBarrier(std::unique_ptr<FContactMaterialRef> Native)
	: NativeRef(std::move(Native))
{
}

FContactMaterialBarrier::~FContactMaterialBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FContactMaterialRef.
}

bool FContactMaterialBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FContactMaterialRef* FContactMaterialBarrier::GetNative()
{
	return NativeRef.get();
}

const FContactMaterialRef* FContactMaterialBarrier::GetNative() const
{
	return NativeRef.get();
}

void FContactMaterialBarrier::AllocateNative(
	const FShapeMaterialBarrier* Material1, const FShapeMaterialBarrier* Material2)
{
	check(!HasNative());

	/// \note AGX seems OK with native materials being null. Falls back on internal default
	/// material.

	agx::MaterialRef NativeMaterial1 =
		Material1 && Material1->HasNative() ? Material1->GetNative()->Native : nullptr;
	agx::MaterialRef NativeMaterial2 =
		Material2 && Material2->HasNative() ? Material2->GetNative()->Native : nullptr;

	NativeRef->Native = new agx::ContactMaterial(NativeMaterial1, NativeMaterial2);
}

void FContactMaterialBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FContactMaterialBarrier::SetFrictionSolveType(int32 SolveType)
{
	check(HasNative());

	agx::FrictionModel* NativeFrictionModel = NativeRef->Native->getFrictionModel();

	if (!NativeFrictionModel) // seems friction model can be null
	{
		// Need a friction model to set solve type. Create the default friction model.
		NativeFrictionModel = new agx::IterativeProjectedConeFriction();
		NativeRef->Native->setFrictionModel(NativeFrictionModel);
	}

	NativeFrictionModel->setSolveType(ConvertSolveTypeToAgx(SolveType));
}

int32 FContactMaterialBarrier::GetFrictionSolveType() const
{
	check(HasNative());

	if (agx::FrictionModel* NativeFrictionModel = NativeRef->Native->getFrictionModel())
	{
		return ConvertSolveTypeToUnreal(NativeFrictionModel->getSolveType());
	}
	else
	{
		return ConvertSolveTypeToUnreal(agx::FrictionModel::NOT_DEFINED);
	}
}

void FContactMaterialBarrier::SetFrictionModel(int32 FrictionModel)
{
	check(HasNative());

	agx::FrictionModelRef NativeFrictionModel = ConvertFrictionModelToAgx(FrictionModel);

	if (NativeFrictionModel)
	{
		if (agx::FrictionModel* PreviousModel = NativeRef->Native->getFrictionModel())
		{
			// Keep solve type from previous model.
			NativeFrictionModel->setSolveType(PreviousModel->getSolveType());
		}
	}

	NativeRef->Native->setFrictionModel(NativeFrictionModel); // seems friction model can be null
}

int32 FContactMaterialBarrier::GetFrictionModel() const
{
	check(HasNative());

	return ConvertFrictionModelToUnreal(NativeRef->Native->getFrictionModel());
}

void FContactMaterialBarrier::SetRestitution(double Restitution)
{
	check(HasNative());
	NativeRef->Native->setRestitution(Restitution);
}

double FContactMaterialBarrier::GetRestitution() const
{
	check(HasNative());
	return NativeRef->Native->getRestitution();
}

void FContactMaterialBarrier::SetSurfaceFrictionEnabled(bool bEnabled)
{
	check(HasNative());
	NativeRef->Native->setEnableSurfaceFriction(bEnabled);
}

bool FContactMaterialBarrier::GetSurfaceFrictionEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceFrictionEnabled();
}

void FContactMaterialBarrier::SetFrictionCoefficient(
	double Coefficient, bool bPrimaryDirection, bool bSecondaryDirection)
{
	check(HasNative());
	agx::ContactMaterial::FrictionDirection NativeDirection =
		ConvertDirectionToAgx(bPrimaryDirection, bSecondaryDirection);
	NativeRef->Native->setFrictionCoefficient(Coefficient, NativeDirection);
}

double FContactMaterialBarrier::GetFrictionCoefficient(
	bool bPrimaryDirection, bool bSecondaryDirection) const
{
	check(HasNative());
	agx::ContactMaterial::FrictionDirection NativeDirection =
		ConvertDirectionToAgx(bPrimaryDirection, bSecondaryDirection);
	return NativeRef->Native->getFrictionCoefficient(NativeDirection);
}

void FContactMaterialBarrier::SetSurfaceViscosity(
	double Viscosity, bool bPrimaryDirection, bool bSecondaryDirection)
{
	check(HasNative());
	agx::ContactMaterial::FrictionDirection NativeDirection =
		ConvertDirectionToAgx(bPrimaryDirection, bSecondaryDirection);
	NativeRef->Native->setSurfaceViscosity(Viscosity, NativeDirection);
}

double FContactMaterialBarrier::GetSurfaceViscosity(
	bool bPrimaryDirection, bool bSecondaryDirection) const
{
	check(HasNative());
	agx::ContactMaterial::FrictionDirection NativeDirection =
		ConvertDirectionToAgx(bPrimaryDirection, bSecondaryDirection);
	return NativeRef->Native->getSurfaceViscosity(NativeDirection);
}

void FContactMaterialBarrier::SetAdhesion(double AdhesiveForce, double AdhesiveOverlap)
{
	check(HasNative());
	NativeRef->Native->setAdhesion(AdhesiveForce, ConvertDistanceToAgx<agx::Real>(AdhesiveOverlap));
}

double FContactMaterialBarrier::GetAdhesiveForce() const
{
	check(HasNative());
	return NativeRef->Native->getAdhesion();
}

double FContactMaterialBarrier::GetAdhesiveOverlap() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getAdhesiveOverlap());
}

void FContactMaterialBarrier::SetYoungsModulus(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->setYoungsModulus(YoungsModulus);
}

double FContactMaterialBarrier::GetYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getYoungsModulus();
}

void FContactMaterialBarrier::SetSpookDamping(double SpookDamping)
{
	check(HasNative());
	NativeRef->Native->setDamping(SpookDamping);
}

double FContactMaterialBarrier::GetSpookDamping() const
{
	check(HasNative());
	return NativeRef->Native->getDamping();
}

void FContactMaterialBarrier::SetMinMaxElasticRestLength(
	double MinElasticRestLength, double MaxElasticRestLength)
{
	check(HasNative());

	NativeRef->Native->setMinMaxElasticRestLength(
		ConvertDistanceToAgx<agx::Real>(MinElasticRestLength),
		ConvertDistanceToAgx<agx::Real>(MaxElasticRestLength));
}

double FContactMaterialBarrier::GetMinElasticRestLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getMinElasticRestLength());
}

double FContactMaterialBarrier::GetMaxElasticRestLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getMaxElasticRestLength());
}

void FContactMaterialBarrier::SetContactReductionMode(int32 ReductionMode)
{
	check(HasNative());
	agx::ContactMaterial::ContactReductionMode NativeMode =
		ConvertReductionModeToAgx(ReductionMode);
	NativeRef->Native->setContactReductionMode(NativeMode);
}

int32 FContactMaterialBarrier::GetContactReductionMode() const
{
	check(HasNative());
	agx::ContactMaterial::ContactReductionMode NativeMode =
		NativeRef->Native->getContactReductionMode();
	return ConvertReductionModeToUnreal(NativeMode);
}

void FContactMaterialBarrier::SetContactReductionBinResolution(uint8 BinResolution)
{
	check(HasNative());
	NativeRef->Native->setContactReductionBinResolution(BinResolution);
}

uint8 FContactMaterialBarrier::GetContactReductionBinResolution() const
{
	check(HasNative());
	return NativeRef->Native->getContactReductionBinResolution();
}

void FContactMaterialBarrier::SetUseContactAreaApproach(bool bUse)
{
	check(HasNative());
	NativeRef->Native->setUseContactAreaApproach(bUse);
}

bool FContactMaterialBarrier::GetUseContactAreaApproach() const
{
	check(HasNative());
	return NativeRef->Native->getUseContactAreaApproach();
}

namespace
{
	agx::Material* GetMaterial(agx::ContactMaterial& Native, int Index)
	{
		check(Index == 1 || Index == 2);
		const agx::Material* Material = Index == 1 ? Native.getMaterial1() : Native.getMaterial2();
		// const_cast because there is no way to get a non-const Material from a ContactMaterial.
		return const_cast<agx::Material*>(Material);
	}

	FShapeMaterialBarrier GetMaterial(FContactMaterialRef& NativeRef, int Index)
	{
		agx::Material* Material = GetMaterial(*NativeRef.Native, Index);
		return AGXBarrierFactories::CreateShapeMaterialBarrier(Material);
	}
}

FShapeMaterialBarrier FContactMaterialBarrier::GetMaterial1() const
{
	check(HasNative());
	return ::GetMaterial(*NativeRef, 1);
}

FShapeMaterialBarrier FContactMaterialBarrier::GetMaterial2() const
{
	check(HasNative());
	return ::GetMaterial(*NativeRef, 2);
}
