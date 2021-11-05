#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/Guid.h"

struct FAGX_CustomVersion
{
	// Important: Do not remove or change the order of enum literals if those have been released
	// publicly. Doing so will break backward compatibility for anyone using that release.
	enum Type
	{
		// Before any version changes were made.
		BeforeCustomVersionWasAdded = 0,

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
