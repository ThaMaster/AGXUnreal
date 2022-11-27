// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/EngineVersion.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Tests/AutomationCommon.h"
#include "UObject/Package.h"
#include "UObject/PackageFileSummary.h"
#include "UObject/UObjectGlobals.h"

namespace RealInMaterialsBackwardsCompatibilitySpec_helpers
{
// We would like to have a way to check the AGX Custom Version of an asset on disk. These are a few
// attempts, but I have not been able to get any of them to work. Hopefully only a minor tweak is
// needed to fix any of them. Until we figure out what that fix is this code will remain disabled,
// to ensure it is not called from anywhere, and other means to check asset validity will be
// required.
#if 0
	/**
	 * Read the AGX_CustomVersion stored in the given asset at the given Content Browser path.
	 */
	int32 GetSavedAGXCustomVersion(const FString& PackagePath)
	{
// This was an attempt to read the asset's version by opening it with an FArchive and calling
// FArchive::CustomVer. It failedbecause I always get the currently running engine's version, not
// the version of the asset.
#if 0
		UE_LOG(LogAGX, Warning, TEXT("Using Package.GetCustomVersionContainer."));
		const FString FilePath = FPaths::ConvertRelativePathToFull(
			FPackageName::LongPackageNameToFilename(
				PackagePath, FPackageName::GetAssetPackageExtension()));
		UE_LOG(LogAGX, Warning, TEXT("Using FArchive::CustomVer."));
		TUniquePtr<FArchive> Archive {IFileManager::Get().CreateFileReader(*FilePath)};
		check(Archive != nullptr);
		Archive->UsingCustomVersion(FAGX_CustomVersion::GUID);
		const int32 SavedVersion = Archive->CustomVer(FAGX_CustomVersion::GUID);
		return SavedVersion;
#endif

// This was an attempt to read the asset's version by reading the asset header from the file and
// getting the Custom Version Container. It failed because the Custom Version Container isn't
// initialized, i.e. we get a nullptr.
#if 0
		UE_LOG(LogAGX, Warning, TEXT("Using Summary.GetCustomVersionContainer."));
		UE_LOG(LogAGX, Warning, TEXT("Using Package.GetCustomVersionContainer."));
		const FString FilePath = FPaths::ConvertRelativePathToFull(
			FPackageName::LongPackageNameToFilename(
				PackagePath, FPackageName::GetAssetPackageExtension()));
		TUniquePtr<FArchive> Archive {IFileManager::Get().CreateFileReader(*FilePath)};
		FPackageFileSummary Summary;
		*Archive << Summary;
		const FCustomVersionContainer& CustomVersions = Summary.GetCustomVersionContainer();
		const FCustomVersion* CustomVersion = CustomVersions.GetVersion(FAGX_CustomVersion::GUID);
		check(CustomVersion != nullptr);
		const int32 SavedVersion = CustomVersion->Version;
		return SavedVersion;
#endif

// This was an attempt to read the asset's version by opening the Package and checking the Linker
// Custom Version. It fails because the Custom Version Container isn't initialized, i.e. we get a
// nullptr.
#if 0
#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#else
		UPackage* Package = CreatePackage(*PackagePath);
#endif
		FCustomVersionContainer& CustomVersions = Package->LinkerCustomVersion;
		const FCustomVersion* CustomVersion = CustomVersions.GetVersion(FAGX_CustomVersion::GUID);
		check(CustomVersion != nullptr);
		const int32 SavedVersion = CustomVersion->Version;
		return SavedVersion;
#endif

		// Until we have a working implementation of GetSavedAGXCustomVersion the best we can do
		// is ensure that this function is never called.
		checkNoEntry();
	}
#endif

	void CheckAssetAGXCustomVersion(const FString& PackagePath, FAutomationTestBase& Test)
	{
#if 0
		const int32 SavedVersion = GetSavedAGXCustomVersion(PackagePath);
		Test.TestEqual(
			"The asset should be saved from a version earlier than when scientific "
			"notation support was added to Materials.",
			SavedVersion, FAGX_CustomVersion::ScientificNotationInMaterials - 1);
#else
		// Until we have a working implementation of GetSavedAGXCustomVersion the best we can do
		// is ensure that this function is never called.
		checkNoEntry();
#endif
	}

	template <typename MaterialT>
	MaterialT* LoadMaterialAsset(const FString& PackagePath, const FString& ObjectName)
	{
		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		// ObjectPath is either FString or FSoftObjectPath depending on Engine version.
		const auto ObjectPath = [&PackagePath, &ObjectName]() {
			const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *ObjectName);
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			return FName(ObjectPath);
#else
			return FSoftObjectPath(ObjectPath);
#endif
		}();
		FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(ObjectPath);
		check(AssetData.IsValid());
		MaterialT* Material = Cast<MaterialT>(AssetData.GetAsset());
		check(Material != nullptr);
		return Material;
	}
}

// clang-format off

/**
 * Unit test that ensures that we can load Material assets that was saved before the switch from
 * double to FAGX_Real for the Properties.
 */
BEGIN_DEFINE_SPEC(
	FFAGX_RealInMaterialsBackwardsCompatibilitySpec,
	"AGXUnreal.Editor.BackwardsCompatibility.FAGX_RealInMaterials", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FFAGX_RealInMaterialsBackwardsCompatibilitySpec)

void FFAGX_RealInMaterialsBackwardsCompatibilitySpec::Define()
{
	using namespace RealInMaterialsBackwardsCompatibilitySpec_helpers;

	Describe("Loading Shape Materials with doubles", [this]()
	{
		It("should convert double to FAGX_Real", [this]()
		{
			const FString ObjectName {TEXT("AGX_SM_PreAGXReal")};
			const FString PackagePath =
				FString::Printf(TEXT("/Game/Tests/BackwardsCompatibility/%s"), *ObjectName);
#if 0
			CheckAssetAGXCustomVersion(PackagePath, *this);
#else
			AgxAutomationCommon::CheckAssetMD5Checksum(PackagePath, TEXT("2113ad88f842ea8c583bd8b037b6007b"), *this);
#endif
			UAGX_ShapeMaterial* ShapeMaterial = LoadMaterialAsset<UAGX_ShapeMaterial>(PackagePath, ObjectName);
			TestEqual(
				TEXT("The shape material should have restored density"),
				ShapeMaterial->Bulk.Density, 1100000.0);
			TestEqual(
				TEXT("The shape material should have restored Young's modulus"),
				ShapeMaterial->Bulk.YoungsModulus, 1200000.0);
			TestEqual(
				TEXT("The shape material should have restored Viscosity"),
				ShapeMaterial->Bulk.Viscosity, 1300000.0);
			TestEqual(
				TEXT("The shape material should have restored Spook damping"),
				ShapeMaterial->Bulk.SpookDamping, 1400000.0);
			TestEqual(
				TEXT("The shape material should have restored min elastic rest length"),
				ShapeMaterial->Bulk.MinElasticRestLength, 1500000.0);
			TestEqual(
				TEXT("The shape material should have restored max elastic rest length"),
				ShapeMaterial->Bulk.MaxElasticRestLength, 1600000.0);
			TestEqual(
				TEXT("The shape material should have restored Roughness"),
				ShapeMaterial->Surface.Roughness, 1700000.0);
			TestEqual(
				TEXT("The shape material should have restored viscosity"),
				ShapeMaterial->Surface.Viscosity, 1800000.0);
			TestEqual(
				TEXT("The shape material should have restored adhesive force"),
				ShapeMaterial->Surface.AdhesiveForce, 1900000.0);
			TestEqual(
				TEXT("The shape material should have restored adhesive overlap"),
				ShapeMaterial->Surface.AdhesiveOverlap, 1110000.0);
			TestEqual(
				TEXT("The shape material should have restored Young's modulus stretch"),
				ShapeMaterial->Wire.YoungsModulusStretch, 1210000.0);
			TestEqual(
				TEXT("The shape material should have restored Spook damping stretch"),
				ShapeMaterial->Wire.SpookDampingStretch, 1310000.0);
			TestEqual(
				TEXT("The shape material should have restored Young's modulus bend"),
				ShapeMaterial->Wire.YoungsModulusBend, 1410000.0);
			TestEqual(
				TEXT("The shape material should have restored Spook damping bend"),
				ShapeMaterial->Wire.SpookDampingBend, 1510000.0);
		});
	});

	Describe("Loading Contact Materials with doubles", [this]()
	{
		It("should convert double to FAGX_Real", [this]()
		{
			const FString ObjectName {TEXT("AGX_CM_PreAGXReal")};
			const FString PackagePath =
				FString::Printf(TEXT("/Game/Tests/BackwardsCompatibility/%s"), *ObjectName);

#if 0
			CheckAssetAGXCustomVersion(PackagePathh, *this);
#else
			AgxAutomationCommon::CheckAssetMD5Checksum(PackagePath, TEXT("0e6c1159d5b5b0bd0dabdc311a2e361e"), *this);
#endif
			UAGX_ContactMaterial* ContactMaterial = LoadMaterialAsset<UAGX_ContactMaterial>(PackagePath, ObjectName);
			TestEqual(
				TEXT("The contact material should have restored friction coefficient"),
				ContactMaterial->FrictionCoefficient, 1000000.0);
			TestEqual(
				TEXT("The contact material should have restored secondary friction coefficient"),
				ContactMaterial->SecondaryFrictionCoefficient, 2000000.0);
			TestEqual(
				TEXT("The contact material should have restored surface viscosity"),
				ContactMaterial->SurfaceViscosity, 3000000.0);
			TestEqual(
				TEXT("The contact material should have restored secondary surface viscosity"),
				ContactMaterial->SecondarySurfaceViscosity, 4000000.0);
			TestEqual(
				TEXT("The contact material should have restored Restitution"),
				ContactMaterial->Restitution, 5000000.0);
			TestEqual(
				TEXT("The contact material should have restored Young's modulus"),
				ContactMaterial->YoungsModulus, 6000000.0);
			TestEqual(
				TEXT("The contact material should have restored Spook damping"),
				ContactMaterial->SpookDamping, 7000000.0);
			TestEqual(
				TEXT("The contact material should have restored adhesive force"),
				ContactMaterial->AdhesiveForce, 8000000.0);
			TestEqual(
				TEXT("The contact material should have restored adhesive overlap"),
				ContactMaterial->AdhesiveOverlap, 9000000.0);
		});
	});

	Describe("Loading Terrain Materials with doubles", [this]()
	{
		It("should convert double to FAGX_Real", [this]()
		{
			const FString ObjectName {TEXT("AGX_TM_PreAGXReal")};
			const FString PackagePath =
				FString::Printf(TEXT("/GAme/Tests/BackwardsCompatibility/%s"), *ObjectName);
#if 0
			CheckAssetAGXCustomVersion(PackagePathh, *this);
#else
			AgxAutomationCommon::CheckAssetMD5Checksum(PackagePath, TEXT("13761b5e4237e63665da78201924246a"), *this);
#endif
			UAGX_TerrainMaterial* TerrainMaterial = LoadMaterialAsset<UAGX_TerrainMaterial>(PackagePath, ObjectName);
			TestTrue(
				TEXT("The terrain material should have restored adhesion overlap factor"),
				TerrainMaterial->TerrainBulk.AdhesionOverlapFactor == 0.000001);
			TestTrue(
				TEXT("The terrain material should have restored cohesion"),
				TerrainMaterial->TerrainBulk.Cohesion == 0.000002);
			TestTrue(
				TEXT("The terrain material should have restored density"),
				TerrainMaterial->TerrainBulk.Density == 0.000003);
			TestTrue(
				TEXT("The terrain material should have restored dilatancy angle"),
				TerrainMaterial->TerrainBulk.DilatancyAngle == 0.000004);
			TestTrue(
				TEXT("The terrain material should have restored friction angle"),
				TerrainMaterial->TerrainBulk.FrictionAngle == 0.000005);
			TestTrue(
				TEXT("The terrain material should have restored max density"),
				TerrainMaterial->TerrainBulk.MaxDensity == 0.000006);
			TestTrue(
				TEXT("The terrain material should have restored Poisson's ration"),
				TerrainMaterial->TerrainBulk.PoissonsRatio == 0.000007);
			TestTrue(
				TEXT("The terrain material should have restored swell factor"),
				TerrainMaterial->TerrainBulk.SwellFactor == 0.000008);
			TestTrue(
				TEXT("The terrain material should have restored Young's modulus"),
				TerrainMaterial->TerrainBulk.YoungsModulus == 0.000009);
			TestTrue(
				TEXT("The terrain material should have restored angle of repose compaction rate"),
				TerrainMaterial->TerrainCompaction.AngleOfReposeCompactionRate == 0.000011);
			TestTrue(
				TEXT("The terrain material should have restored phi 0"),
				TerrainMaterial->TerrainCompaction.BankStatePhi0 == 0.000012);
			TestTrue(
				TEXT("The terrain material should have restored compaction time relaxation constant"),
				TerrainMaterial->TerrainCompaction.CompactionTimeRelaxationConstant == 0.000013);
			TestTrue(
				TEXT("The terrain material should have restored compression index"),
				TerrainMaterial->TerrainCompaction.CompressionIndex == 0.000014);
			TestTrue(
				TEXT("The terrain material should have restored K E"),
				TerrainMaterial->TerrainCompaction.HardeningConstantKe == 0.000015);
			TestTrue(
				TEXT("The terrain material should have restored N E"),
				TerrainMaterial->TerrainCompaction.HardeningConstantNe == 0.000016);
			TestTrue(
				TEXT("The terrain material should have restored preconsolidation stress"),
				TerrainMaterial->TerrainCompaction.PreconsolidationStress == 0.000017);
			TestTrue(
				TEXT("The terrain material should have restored stress cut off fraction"),
				TerrainMaterial->TerrainCompaction.StressCutOffFraction == 0.000018);
			TestTrue(
				TEXT("The shape material should have restored Roughness"),
				TerrainMaterial->Surface.Roughness == 0.000019);
			TestTrue(
				TEXT("The shape material should have restored viscosity"),
				TerrainMaterial->Surface.Viscosity == 0.000021);
			TestTrue(
				TEXT("The shape material should have restored adhesive force"),
				TerrainMaterial->Surface.AdhesiveForce == 0.000022);
			TestTrue(
				TEXT("The shape material should have restored adhesive overlap"),
				TerrainMaterial->Surface.AdhesiveOverlap == 0.000023);
			TestTrue(
				TEXT("The shape material should have restored Young's modulus stretch"),
				TerrainMaterial->Wire.YoungsModulusStretch == 0.000024);
			TestTrue(
				TEXT("The shape material should have restored Spook damping stretch"),
				TerrainMaterial->Wire.SpookDampingStretch == 0.000025);
			TestTrue(
				TEXT("The shape material should have restored Young's modulus bend"),
				TerrainMaterial->Wire.YoungsModulusBend == 0.000026);
			TestTrue(
				TEXT("The shape material should have restored Spook damping bend"),
				TerrainMaterial->Wire.SpookDampingBend == 0.000027);
		});
	});
}

// clang-format on
