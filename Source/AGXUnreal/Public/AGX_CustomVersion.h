#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/Guid.h"

struct FAGX_CustomVersion
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

		// < -----new versions can be added above this line----->
		VersionPlusOne,

		LatestVersion = VersionPlusOne - 1
	};

	const static FGuid GUID;

private:
	FAGX_CustomVersion()
	{
	}
};
