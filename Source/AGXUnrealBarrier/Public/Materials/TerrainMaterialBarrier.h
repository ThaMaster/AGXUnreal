#pragma once

#include <memory>

#include "Containers/UnrealString.h"

struct FTerrainMaterialRef;

/**
 * Acts as an interface to a native AGX Terrain Material, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FTerrainMaterialBarrier
{
public:
	FTerrainMaterialBarrier();
	virtual ~FTerrainMaterialBarrier();

	bool HasNative() const;
	FTerrainMaterialRef* GetNative();
	const FTerrainMaterialRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	void SetName(const FString& Name);
	FString GetName() const;

	void SetDensity(double Density);
	double GetDensity() const;

private:
	FTerrainMaterialBarrier(const FTerrainMaterialBarrier&) = delete;
	void operator=(const FTerrainMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FTerrainMaterialRef> NativeRef;
};
