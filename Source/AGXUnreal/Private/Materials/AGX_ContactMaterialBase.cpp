#include "Materials/AGX_ContactMaterialBase.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_ContactMaterialInstance.h"
#include "Materials/ContactMaterialBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_ContactMaterialInstance* UAGX_ContactMaterialBase::GetOrCreateInstance(
	UWorld* PlayingWorld, UAGX_ContactMaterialBase*& Property)
{
	if (Property == nullptr || PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		return nullptr;
	}

	UAGX_ContactMaterialInstance* Instance = Property->GetOrCreateInstance(PlayingWorld);

	if (Instance != Property)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("UAGX_ContactMaterialBase::GetOrCreateInstance is swapping a property (to \"%s\" "
				 "from \"%s\")."),
			*GetNameSafe(Instance), *GetNameSafe(Property));

		Property = Instance;
	}

	return Instance;
}

void UAGX_ContactMaterialBase::CopyFrom(const FContactMaterialBarrier* Source)
{
	if (Source)
	{
		ContactSolver = static_cast<EAGX_ContactSolver>(Source->GetFrictionSolveType());

		ContactReduction = FAGX_ContactMaterialReductionMode();
		ContactReduction.Mode =
			static_cast<EAGX_ContactReductionMode>(Source->GetContactReductionMode());
		ContactReduction.BinResolution = Source->GetContactReductionBinResolution();

		MechanicsApproach = FAGX_ContactMaterialMechanicsApproach();
		MechanicsApproach.bUseContactAreaApproach = Source->GetUseContactAreaApproach();
		MechanicsApproach.MinElasticRestLength = Source->GetMinElasticRestLength();
		MechanicsApproach.MaxElasticRestLength = Source->GetMaxElasticRestLength();

		FrictionModel = static_cast<EAGX_FrictionModel>(Source->GetFrictionModel());
		bSurfaceFrictionEnabled = Source->GetSurfaceFrictionEnabled();
		FrictionCoefficient = Source->GetFrictionCoefficient(true, false);
		SecondaryFrictionCoefficient = Source->GetFrictionCoefficient(false, true);
		bUseSecondaryFrictionCoefficient = FrictionCoefficient != SecondaryFrictionCoefficient;
		SurfaceViscosity = Source->GetSurfaceViscosity(true, false);
		SecondarySurfaceViscosity = Source->GetSurfaceViscosity(false, true);
		bUseSecondarySurfaceViscosity = SurfaceViscosity != SecondarySurfaceViscosity;
		Restitution = Source->GetRestitution();
		YoungsModulus = Source->GetYoungsModulus();
		Damping = Source->GetDamping();
		AdhesiveForce = Source->GetAdhesiveForce();
		AdhesiveOverlap = Source->GetAdhesiveOverlap();
	}
}

UAGX_ContactMaterialBase::UAGX_ContactMaterialBase()
	: ContactSolver(EAGX_ContactSolver::Split)
	, ContactReduction()
	, // defaults defined in struct's constructor
	MechanicsApproach()
	, // defaults defined in struct's constructor
	FrictionModel(EAGX_FrictionModel::IterativeProjectedConeFriction)
	, bSurfaceFrictionEnabled(true)
	, FrictionCoefficient(0.25 / (2 * 0.3))
	, SecondaryFrictionCoefficient(0.25 / (2 * 0.3))
	, bUseSecondaryFrictionCoefficient(false)
	, SurfaceViscosity(5.0E-9)
	, SecondarySurfaceViscosity(5.0E-9)
	, bUseSecondarySurfaceViscosity(false)
	, Restitution(0.5)
	, YoungsModulus(2.0 / 5.0E-9)
	, Damping(4.5 / 60.0)
	, AdhesiveForce(0.0)
	, AdhesiveOverlap(0.0)
{
	// See agx\src\agx\Material.cpp for default values
}

UAGX_ContactMaterialBase::~UAGX_ContactMaterialBase()
{
}

#define COPY_MAT_PROPERTY(Source, Name) \
	{                                   \
		Name = Source->Name;            \
	}

void UAGX_ContactMaterialBase::CopyFrom(const UAGX_ContactMaterialBase* Source)
{
	if (Source)
	{
		/// \todo Is there a way to make this in a more implicit way? Easy to forget these when
		/// adding properties.
		COPY_MAT_PROPERTY(Source, Material1);
		COPY_MAT_PROPERTY(Source, Material2);

		COPY_MAT_PROPERTY(Source, ContactSolver);
		COPY_MAT_PROPERTY(Source, ContactReduction);
		COPY_MAT_PROPERTY(Source, MechanicsApproach);

		COPY_MAT_PROPERTY(Source, FrictionModel);
		COPY_MAT_PROPERTY(Source, bSurfaceFrictionEnabled);
		COPY_MAT_PROPERTY(Source, FrictionCoefficient);
		COPY_MAT_PROPERTY(Source, SecondaryFrictionCoefficient);
		COPY_MAT_PROPERTY(Source, bUseSecondaryFrictionCoefficient);
		COPY_MAT_PROPERTY(Source, SurfaceViscosity);
		COPY_MAT_PROPERTY(Source, SecondarySurfaceViscosity);
		COPY_MAT_PROPERTY(Source, bUseSecondarySurfaceViscosity);

		COPY_MAT_PROPERTY(Source, Restitution);
		COPY_MAT_PROPERTY(Source, YoungsModulus);
		COPY_MAT_PROPERTY(Source, Damping);
		COPY_MAT_PROPERTY(Source, AdhesiveForce);
		COPY_MAT_PROPERTY(Source, AdhesiveOverlap);
	}
}

#undef COPY_MAT_PROPERTY
