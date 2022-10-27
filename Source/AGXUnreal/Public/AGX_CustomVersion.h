// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/Guid.h"

class FArchive;

struct AGXUNREAL_API FAGX_CustomVersion
{
	// Important: Do not remove or change the order of enum literals if those have been released
	// publicly. Doing so will break backward compatibility for anyone using that release.
	// There is an exception: if a backwards compatibility "reset" is wanted, for example when
	// moving to a new major release, all enum literals in this Type enum except the first and the
	// two last ones can be removed. When doing such a "reset", remember to set a newly randomly
	// generated GUID in AGX_CustomVersion.cpp also. That GUID together with these enum literal
	// values are what makes a certain version completely unique.
	enum Type
	{
		// Before any version changes were made.
		BeforeCustomVersionWasAdded = 0,

		ConstraintsStoreComplianceInsteadOfElasticity,

		// To better support large and small number we replace float/double with FAGX_Real in Shape-
		// and ContactMaterial.
		ScientificNotationInMaterials,

		// The ...MaterialBase, ... MaterialInstance, ...MaterialAsset have all been combined into a
		// single class for ShapeMaterials, TerrainMaterials and ContactMaterials respectively.
		BaseAssetInstanceMaterialClassesAreOneClass,

		// <----- New versions can be added above this line. ----->
		VersionPlusOne,

		LatestVersion = VersionPlusOne - 1
	};

	const static FGuid GUID;

private:
	FAGX_CustomVersion()
	{
	}
};

/**
 * Determine if we are loading from an archive that is older than the given version, which means
 * that we should perform any transformations necessary to bring the state from the pre-version
 * format to the post-version format.
 *
 * @param Archive The archive we're serializing from/to.
 * @param Version The version to compare with.
 * @return True if we should do an upgrade.
 */
bool ShouldUpgradeTo(const FArchive& Archive, FAGX_CustomVersion::Type Version);
