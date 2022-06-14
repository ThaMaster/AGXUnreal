// Copyright 2022, Algoryx Simulation AB.

#include "Materials/ContactMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "AGXBarrierFactories.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/FrictionModel.h>
#include <agx/OrientedFrictionModels.h>
#include <agx/RigidBody.h>
#include "EndAGXIncludes.h"

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

		// \note The oriented friction models have no public default constructor,
		//       so just set temporary default values and depend on that they are overwritten
		//       by subsequent setter calls.
		static const agx::Vec3 DefaultPrimaryDir = agx::Vec3::X_AXIS();

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
			case 4:
				return new agx::OrientedBoxFrictionModel(nullptr, DefaultPrimaryDir);
			case 5:
				return new agx::OrientedScaleBoxFrictionModel(nullptr, DefaultPrimaryDir);
			case 6:
				return new agx::OrientedIterativeProjectedConeFrictionModel(
					nullptr, DefaultPrimaryDir);
			case 7:
				return new agx::ConstantNormalForceOrientedBoxFrictionModel(
					0, nullptr, DefaultPrimaryDir);
			default:
				check(!"ConvertFrictionModelToAgx received unsupported value");
				return nullptr;
		}
	}

	int32 ConvertFrictionModelToUnreal(const agx::FrictionModel* FrictionModel)
	{
		// Output value refers to EAGX_FrictionModel in AGX_ContactMaterialEnums.h

		// \note Order below is important because some types below inherit from others.
		//       Hierarchically deep types must be cast-tested before their ancestor types!
		// \todo Consider using typeid() to compare exact types instead.

		if (dynamic_cast<const agx::ConstantNormalForceOrientedBoxFrictionModel*>(FrictionModel))
		{
			return 7;
		}
		if (dynamic_cast<const agx::OrientedIterativeProjectedConeFrictionModel*>(FrictionModel))
		{
			return 6;
		}
		else if (dynamic_cast<const agx::OrientedScaleBoxFrictionModel*>(FrictionModel))
		{
			return 5;
		}
		else if (dynamic_cast<const agx::OrientedBoxFrictionModel*>(FrictionModel))
		{
			return 4;
		}
		else if (dynamic_cast<const agx::IterativeProjectedConeFriction*>(FrictionModel))
		{
			return 3;
		}
		else if (dynamic_cast<const agx::ScaleBoxFrictionModel*>(FrictionModel))
		{
			return 2;
		}
		else if (dynamic_cast<const agx::BoxFrictionModel*>(FrictionModel))
		{
			return 1;
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

bool FContactMaterialBarrier::SetNormalForceMagnitude(double NormalForceMagnitude)
{
	check(HasNative());

	if (auto* FrictionModel = dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(
			NativeRef->Native->getFrictionModel()))
	{
		FrictionModel->setNormalForceMagnitude(NormalForceMagnitude);
		return true;
	}
	else
	{
		// \todo If FrictionModel is set AFTER this function call, we could cache the
		// NormalForceMagnitude here and write it when FrictionModel changes to appropriate type.
		// That would make the code less prone to problems caused by function call order.
		UE_LOG(
			LogAGX, Warning,
			TEXT("Failed to set NormalForceMagnitude on native ContactMaterial because its "
				 "FrictionModel has not been set to ConstantNormalForceOrientedBoxFrictionModel."));
		return false;
	}
}

bool FContactMaterialBarrier::GetNormalForceMagnitude(double& NormalForceMagnitude) const
{
	check(HasNative());

	if (auto* FrictionModel = dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(
			NativeRef->Native->getFrictionModel()))
	{
		NormalForceMagnitude = FrictionModel->getNormalForceMagnitude();
		return true;
	}
	else
	{
		return false;
	}
}

bool FContactMaterialBarrier::SetEnableScaleNormalForceWithDepth(bool bEnabled)
{
	check(HasNative());

	if (auto* FrictionModel = dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(
			NativeRef->Native->getFrictionModel()))
	{
		FrictionModel->setEnableScaleWithDepth(bEnabled);
		return true;
	}
	else
	{
		// \todo If FrictionModel is set AFTER this function call, we could cache the
		// NormalForceMagnitude here and write it when FrictionModel changes to appropriate type.
		// That would make the code less prone to problems caused by function call order.
		UE_LOG(
			LogAGX, Warning,
			TEXT("Failed to set EnableScaleNormalForceWithDepth on native ContactMaterial because "
				 "its "
				 "FrictionModel has not been set to ConstantNormalForceOrientedBoxFrictionModel."));
		return false;
	}
}

bool FContactMaterialBarrier::GetEnableScaleNormalForceWithDepth(bool& bEnabled) const
{
	check(HasNative());

	if (auto* FrictionModel = dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(
			NativeRef->Native->getFrictionModel()))
	{
		bEnabled = FrictionModel->getEnableScaleWithDepth();
		return true;
	}
	else
	{
		return false;
	}
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

bool FContactMaterialBarrier::SetPrimaryDirection(const FVector& Direction)
{
	check(HasNative());

	agx::FrictionModel* FrictionModel = NativeRef->Native->getFrictionModel();
	agx::Vec3 DirAGX = ConvertVector(Direction.GetSafeNormal());

	// \todo Perhaps we can solve the if-else below in a more elegant way using templates or macros?

	if (auto* OrientedBoxModel = dynamic_cast<agx::OrientedBoxFrictionModel*>(FrictionModel))
	{
		OrientedBoxModel->setPrimaryDirection(DirAGX);
	}
	else if (
		auto* OrientedScaleBoxModel =
			dynamic_cast<agx::OrientedScaleBoxFrictionModel*>(FrictionModel))
	{
		OrientedScaleBoxModel->setPrimaryDirection(DirAGX);
	}
	else if (
		auto* OrientedConeModel =
			dynamic_cast<agx::OrientedIterativeProjectedConeFrictionModel*>(FrictionModel))
	{
		OrientedConeModel->setPrimaryDirection(DirAGX);
	}
	else if (
		auto* OrientedConstantNormalModel =
			dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(FrictionModel))
	{
		OrientedConstantNormalModel->setPrimaryDirection(DirAGX);
	}
	else
	{
		// \todo If FrictionModel is set AFTER this function call, we could cache the
		// Direction vector here and write it when FrictionModel changes to appropriate type.
		// That would make the code less prone to problems caused by function call order.
		UE_LOG(
			LogAGX, Warning,
			TEXT("Failed to set PrimaryDirection on native ContactMaterial because it has "
				 "not been set to use an Oriented Friction Model."));
		return false;
	}
	return true;
}

bool FContactMaterialBarrier::GetPrimaryDirection(FVector& Direction) const
{
	check(HasNative());

	agx::FrictionModel* FrictionModel = NativeRef->Native->getFrictionModel();
	agx::Vec3 DirAGX = agx::Vec3::X_AXIS();

	// \todo Perhaps we can solve the if-else below in a more elegant way using templates or macros?

	if (auto* OrientedBoxModel = dynamic_cast<agx::OrientedBoxFrictionModel*>(FrictionModel))
	{
		DirAGX = OrientedBoxModel->getPrimaryDirection();
	}
	else if (auto* OrientedScaleBoxModel = dynamic_cast<agx::OrientedScaleBoxFrictionModel*>(FrictionModel))
	{
		DirAGX = OrientedScaleBoxModel->getPrimaryDirection();
	}
	else if (
		auto* OrientedConeModel =
			dynamic_cast<agx::OrientedIterativeProjectedConeFrictionModel*>(FrictionModel))
	{
		DirAGX = OrientedConeModel->getPrimaryDirection();
	}
	else if (
		auto* OrientedConstantNormalModel =
			dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(FrictionModel))
	{
		DirAGX = OrientedConstantNormalModel->getPrimaryDirection();
	}
	else
	{
		return false;
	}
	return true;
}

bool FContactMaterialBarrier::SetOrientedFrictionModelReferenceFrame(FRigidBodyBarrier* RigidBody)
{
	check(HasNative());

	// \todo Let the user choose frames other than RigidBody, for example it would be convenient
	//       to be able to use the frame of a Shape, or any UE Scene Component (could be passed to
	//       this method as a rotation parameter relative to the parent Rigid Body, from which a
	//       hidden AGX child frame could be generated and passed to setReferenceFrame below).

	agx::FrictionModel* FrictionModel = NativeRef->Native->getFrictionModel();
	agx::RigidBody* Body = FAGX_AgxDynamicsObjectsAccess::TryGetFrom(RigidBody);
	agx::Frame* Frame = Body ? Body->getFrame() : nullptr;

	// \todo Perhaps we can solve the if-else below in a more elegant way using templates or macros?

	if (auto* OrientedBoxModel = dynamic_cast<agx::OrientedBoxFrictionModel*>(FrictionModel))
	{
		OrientedBoxModel->setReferenceFrame(Frame);
	}
	else if (auto* OrientedScaleBoxModel = dynamic_cast<agx::OrientedScaleBoxFrictionModel*>(FrictionModel))
	{
		OrientedScaleBoxModel->setReferenceFrame(Frame);
	}
	else if (
		auto* OrientedConeModel =
			dynamic_cast<agx::OrientedIterativeProjectedConeFrictionModel*>(FrictionModel))
	{
		OrientedConeModel->setReferenceFrame(Frame);
	}
	else if (
		auto* OrientedConstantNormalModel =
			dynamic_cast<agx::ConstantNormalForceOrientedBoxFrictionModel*>(FrictionModel))
	{
		OrientedConstantNormalModel->setReferenceFrame(Frame);
	}
	else
	{
		// \todo If FrictionModel is set AFTER this function call, we could cache the
		// Direction vector here and write it when FrictionModel changes to appropriate type.
		// That would make the code less prone to problems caused by function call order.
		UE_LOG(
			LogAGX, Warning,
			TEXT("Failed to set ReferenceFrame on native ContactMaterial because it has "
				 "not been set to use an Oriented Friction Model."));
		return false;
	}
	return true;
}

void FContactMaterialBarrier::SetAdhesion(double AdhesiveForce, double AdhesiveOverlap)
{
	check(HasNative());
	NativeRef->Native->setAdhesion(AdhesiveForce, ConvertDistanceToAGX<agx::Real>(AdhesiveOverlap));
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
		ConvertDistanceToAGX<agx::Real>(MinElasticRestLength),
		ConvertDistanceToAGX<agx::Real>(MaxElasticRestLength));
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
