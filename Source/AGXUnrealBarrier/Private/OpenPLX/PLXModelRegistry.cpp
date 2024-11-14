// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLXModelRegistry.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilities.h"

// Standard library includes.
#include <limits>

FPLXModelRegistry::FPLXModelRegistry()
	: Native(std::make_unique<FPLXModelData>())
{
}

FPLXModelRegistry::~FPLXModelRegistry()
{
}

bool FPLXModelRegistry::HasNative() const
{
	return Native != nullptr;
}

void FPLXModelRegistry::ReleaseNative()
{
	Native = nullptr;
}

namespace FPLXModelRegistry_helpers
{
	FPLXModelRegistry::Handle Convert(size_t Val)
	{
		if (Val > std::numeric_limits<FPLXModelRegistry::Handle>::max())
		{
			// This should never ever happen. It means we have more than
			// int32::max number of models in the world.
			AGX_CHECK(false);
			return std::numeric_limits<FPLXModelRegistry::Handle>::max();
		}

		return static_cast<FPLXModelRegistry::Handle>(Val);
	}

	size_t Convert(FPLXModelRegistry::Handle Val)
	{
		check(Val >= 0);
		return static_cast<size_t>(Val);
	}
}

FPLXModelRegistry::Handle FPLXModelRegistry::Register(
	const FString& PLXFile, FAssemblyRef& Assembly, FSimulationBarrier& Simulation)
{
	check(HasNative());
	check(Simulation.HasNative());

	Handle Handle = GetFrom(PLXFile);

	if (Handle == InvalidHandle) // We have never seen this PLX Model before.
		Handle = PrepareNewModel(PLXFile);

	if (Handle == InvalidHandle)
		return InvalidHandle; // Something went wrong.

	// At this point, we know there exists valid PLXModelData for this PLXModel.
	// We need to add the given Assembly to the base Assembly and (re)-create the
	// InputSignalListener which is shared by all instances of this PLX Model.

	FPLXModelDatum* ModelDatum = GetModelDatum(Handle);
	if (ModelDatum == nullptr)
		return InvalidHandle;

	// Add the given assebly as a sub-assembly to the existing root assembly.
	// This way, the InputSignalListener will correctly find all relevant AGX objects.
	ModelDatum->Assembly->add(Assembly.Native);
	
	if (ModelDatum->InputSignalListener != nullptr) // Has been added before.
		Simulation.GetNative()->Native->remove(ModelDatum->InputSignalListener);

	// The InputSignalListener always needs to be re-created even if it existed before, since now
	// we have added another sub-assembly containing agx-objects it needs to know about.
	ModelDatum->InputSignalListener = new BrickAgx::InputSignalListener(ModelDatum->Assembly);
	Simulation.GetNative()->Native->add(ModelDatum->InputSignalListener);

	return Handle;
}

const FPLXModelDatum* FPLXModelRegistry::GetModelDatum(Handle Handle) const
{
	check(HasNative());
	if (Handle == InvalidHandle)
		return nullptr;

	const size_t Index = FPLXModelRegistry_helpers::Convert(Handle);
	if (Index >= Native->ModelData.size())
		return nullptr;

	return &Native->ModelData[Index];
}

FPLXModelDatum* FPLXModelRegistry::GetModelDatum(Handle Handle)
{
	check(HasNative());
	if (Handle == InvalidHandle)
		return nullptr;

	const size_t Index = FPLXModelRegistry_helpers::Convert(Handle);
	if (Index >= Native->ModelData.size())
		return nullptr;

	return &Native->ModelData[Index];
}

FPLXModelRegistry::Handle FPLXModelRegistry::GetFrom(const FString& PLXFile) const
{
	auto It = KnownModels.find(Convert(PLXFile));
	return It != KnownModels.end() ? It->second : InvalidHandle;
}

FPLXModelRegistry::Handle FPLXModelRegistry::PrepareNewModel(const FString& PLXFile)
{
	// Here we create a new slot in the PLXModelData array with the PLX model tree as well
	// as some other required objects line an agxSDK::Assembly that the InputSignalListener needs.
	AGX_CHECK(GetFrom(PLXFile) == InvalidHandle);

	FPLXModelDatum NewModel;
	NewModel.PLXModel = FPLXUtilities::LoadModel(PLXFile, NewModel.AGXCache);
	if (NewModel.PLXModel == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not read OpenPLX file '%s'. The Log category LogAGXDynamics may include "
				 "more details."),
			*PLXFile);
		return InvalidHandle;
	}

	// OpenPLX's InputSignalListener expects an Assembly with a PowerLine always.
	// Therefore we add one here, even though it's a bit of a hack.
	NewModel.Assembly = new agxSDK::Assembly();
	agxPowerLine::PowerLineRef RequiredDummyPowerLine = new agxPowerLine::PowerLine();
	RequiredDummyPowerLine->setName(agx::Name("BrickPowerLine"));
	NewModel.Assembly->add(RequiredDummyPowerLine);

	const Handle NewHande = FPLXModelRegistry_helpers::Convert(Native->ModelData.size());
	Native->ModelData.emplace_back(std::move(NewModel));
	KnownModels.insert({Convert(PLXFile), NewHande});

	return NewHande;
}
