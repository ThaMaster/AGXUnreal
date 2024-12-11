// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImporterUtilities.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_ObjectUtilities.h"

void FAGX_ImporterUtilities::CopyProperties(
	const UObject& Source, UObject& OutDestination, bool UpdateArchetypeInstances)
{
	UClass* Class = Source.GetClass();

	TArray<UObject*> ArchetypeInstances;
	if (UpdateArchetypeInstances)
		OutDestination.GetArchetypeInstances(ArchetypeInstances);

	for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		// To speed up execution, can we add custom property flags for AGXUnreal properties?
		if (Property && Property->HasAnyPropertyFlags(CPF_Edit))
		{
			const void* SourceValue = Property->ContainerPtrToValuePtr<void>(&Source);
			void* DestValue = Property->ContainerPtrToValuePtr<void>(&OutDestination);
			if (Property->Identical(SourceValue, DestValue))
				continue; // Nothing to do, already equal.

			if (UpdateArchetypeInstances)
			{
				for (UObject* Instance : ArchetypeInstances)
				{
					if (Instance == nullptr)
						continue;

					void* ArchetypeInstanceValue = Property->ContainerPtrToValuePtr<void>(Instance);
					if (Property->Identical(ArchetypeInstanceValue, DestValue)) // In sync; copy!
						Property->CopyCompleteValue(ArchetypeInstanceValue, SourceValue);
				}
			}

			Property->CopyCompleteValue(DestValue, SourceValue);
		}
	}
}
