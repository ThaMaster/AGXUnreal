// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <memory>
#include <string>
#include <unordered_map>

class FSimulationBarrier;

struct FAssemblyRef;
struct FPLXModelData;
struct FPLXModelDatum;

class AGXUNREALBARRIER_API FPLXModelRegistry
{
public:
	using Handle = int32;
	inline static constexpr Handle InvalidHandle = -1;

	FPLXModelRegistry();
	~FPLXModelRegistry();

	bool HasNative() const;
	void ReleaseNative();

	/**
	 * The PLXFile is the absolute path of a OpenPLX model to be loaded.
	 * The Assembly must contain the AGX objects relevant for the specific instance of this OpenPLX
	 * model (there can exist many).
	 * The Simulation is the Simulation object used by AGXUnreal during Play.
	 * 
	 * The Handle returned can be used to later access the loaded OpenPLX model along with related
	 * data. This Handle will be shared by all who register the same PLX file, for example when the
	 * same PLX model is instanced many times in the same world.
	 */
	Handle Register(
		const FString& PLXFile, const FString& UniqueModelInstancePrefix, FAssemblyRef& Assembly,
		FSimulationBarrier& Simulation);

	/**
	 * Important note: the lifetime of the returned FPLXModelDatum is only guaranteed during direct
	 * usage in local scope. Do not store this pointer for later use.
	 */
	const FPLXModelDatum* GetModelDatum(Handle Handle) const;
	FPLXModelDatum* GetModelDatum(Handle Handle);

private:
	FPLXModelRegistry(const FPLXModelRegistry&) = delete;
	void operator=(const FPLXModelRegistry&) = delete;

	Handle GetFrom(const FString& PLXFile) const;
	Handle PrepareNewModel(const FString& PLXFile);

	std::unique_ptr<FPLXModelData> Native;
	std::unordered_map<std::string, Handle> KnownModels;
};
