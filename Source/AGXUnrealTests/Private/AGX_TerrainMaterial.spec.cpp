// Copyright 2024, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Materials/AGX_TerrainMaterial.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"

BEGIN_DEFINE_SPEC(
	FAGX_TerrainMaterialSpec, "AGXUnreal.Spec.TerrainMaterial",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_TerrainMaterialSpec)

namespace AGX_TerrainMaterialSpec_helpers
{
	template <typename CallbackT>
	void VisitProperties(
		FStructProperty* StructProperty, void* StructMemory, const FString& CPPType,
		CallbackT& Callback)
	{
		UScriptStruct* Struct = StructProperty->Struct;
		UE_LOG(
			LogAGX, Warning, TEXT("  Visiting %s properties in %s."), *CPPType,
			*StructProperty->GetName());
		for (TFieldIterator<FProperty> PropertyIt(Struct); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			UE_LOG(
				LogAGX, Warning, TEXT("Found property named %s of type %s."), *Property->GetName(),
				*Property->GetCPPType());
			if (Property->HasAnyPropertyFlags(CPF_Deprecated))
			{
				continue;
			}
			void* PropertyMemory = Property->ContainerPtrToValuePtr<void>(StructMemory);
			if (Property->GetCPPType() == CPPType)
			{
				Callback(PropertyMemory, *Property->GetName());
			}
			else if (FStructProperty* NestedStructProperty = CastField<FStructProperty>(Property))
			{
				VisitProperties(NestedStructProperty, PropertyMemory, CPPType, Callback);
			}
		}
	}

	template <typename CallbackT>
	void VisitProperties(UObject* Object, const FString& CPPType, CallbackT& Callback)
	{
		UClass* Class = Object->GetClass();
		UE_LOG(
			LogAGX, Warning, TEXT("Visiting %s properties in %s."), *CPPType, *Object->GetName());
		for (TFieldIterator<FProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			UE_LOG(
				LogAGX, Warning, TEXT("Found property named %s of type %s."), *Property->GetName(),
				*Property->GetCPPType());
			if (Property->HasAnyPropertyFlags(CPF_Deprecated))
			{
				continue;
			}
			void* PropertyMemory = Property->ContainerPtrToValuePtr<void>(Object);
			if (Property->GetCPPType() == CPPType)
			{
				Callback(PropertyMemory, *Property->GetName());
			}
			else if (FStructProperty* NestedStructProperty = CastField<FStructProperty>(Property))
			{
				VisitProperties(NestedStructProperty, PropertyMemory, CPPType, Callback);
			}
		}
	}

	void SetAllRealPropertiesToIncreasingValues(UObject* Object)
	{
		auto Callback = [NextValue = 0.0](void* Memory, const TCHAR* /*Name*/) mutable
		{
			FAGX_Real* Real = static_cast<FAGX_Real*>(Memory);
			UE_LOG(LogAGX, Warning, TEXT("  Overwriting %f with %f."), Real->Value, NextValue);
			Real->Value = NextValue;
			NextValue = NextValue + 1.0;
		};

		VisitProperties(Object, TEXT("FAGX_Real"), Callback);
	}

	void AssertAllRealPropertiesHaveIncreasingValues(
		UObject* Object, FAGX_TerrainMaterialSpec& Test)
	{
		auto Callback = [NextValue = 0.0, &Test](void* Memory, const TCHAR* Name) mutable
		{
			FAGX_Real* Real = static_cast<FAGX_Real*>(Memory);
			UE_LOG(LogAGX, Warning, TEXT("  Expecting %f, found %f."), NextValue, Real->Value);
			if (!Test.TestEqual(TEXT("A property"), Real->Value, NextValue))
			{
				UE_LOG(
					LogAGX, Warning, TEXT("Failed increasing-values check for property '%s'."),
					Name);
			}
			NextValue = NextValue + 1.0;
		};

		VisitProperties(Object, TEXT("FAGX_Real"), Callback);
	}
}

void FAGX_TerrainMaterialSpec::Define()
{
	using namespace AGX_TerrainMaterialSpec_helpers;
	Describe(
		"When copying properties from one Terrain Material to another",
		[this]()
		{
			It("should copy all properties",
			   [this]()
			   {
				   TObjectPtr<UAGX_TerrainMaterial> Source = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Source Terrain Material"));

				   SetAllRealPropertiesToIncreasingValues(Source);
				   AssertAllRealPropertiesHaveIncreasingValues(Source, *this);

				   TObjectPtr<UAGX_TerrainMaterial> Destination = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Destination Terrain Material"));
				   Destination->CopyTerrainMaterialProperties(Source);
				   AssertAllRealPropertiesHaveIncreasingValues(Destination, *this);
			   });
		});

	Describe(
		"When copying properties from a Terrain Material to a Barrier",
		[this]()
		{
			It("should copy all properties",
			   [this]()
			   {
				   TObjectPtr<UAGX_TerrainMaterial> Source = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Source Terrain Material"));
				   SetAllRealPropertiesToIncreasingValues(Source);

				   // AGX Dynamics puts limits on some properties. Make sure we are within those
				   // limits before creating the native.
				   double BankStatePhi0Test {Source->TerrainCompaction.BankStatePhi0};
				   double BankStatePhi0Agx {1.0};
				   Source->TerrainCompaction.BankStatePhi0 = BankStatePhi0Agx;

				   UWorld* World = UWorld::CreateWorld(
					   EWorldType::Game, false, TEXT("Terrain Material Test World"),
					   GetTransientPackage());

				   TObjectPtr<UAGX_TerrainMaterial> Instance =
					   UAGX_TerrainMaterial::CreateFromAsset(World, Source);
				   FTerrainMaterialBarrier* Barrier = Instance->GetTerrainMaterialNative();

				   TObjectPtr<UAGX_TerrainMaterial> Destination = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Destination Terrain Material"));
				   Destination->CopyFrom(*Barrier);

				   // Special handling for properties that AGX Dynamics doesn't support arbitrary
				   // values for.

				   // Bank State Phi 0 has limits. Do test on the limited value and then set back
				   // to what the generic test harness expects.
				   TestEqual(
					   TEXT("Terrain Material Compaction Bank State Phi 0"),
					   Destination->TerrainCompaction.BankStatePhi0, BankStatePhi0Agx);
				   Destination->TerrainCompaction.BankStatePhi0 = BankStatePhi0Test;

				   // Adhesion Overlap Factor is aliased, both Terrain Bulk and Terrain Particles
				   // has it and they write to the same AGX Dynamics memory. This means that when
				   // we read back from AGX Dynamics we do two reads of the same memory and expect
				   // two different values. That's not going to happen. Hard-copy here so that the
				   // test don't fail.
				   Destination->TerrainBulk.AdhesionOverlapFactor =
					   Source->TerrainBulk.AdhesionOverlapFactor;

				   // Bulk, Surface, and Wire properties, the old remains from when Terrain Material
				   // was a Shape Material, are not copied Barrier -> Asset so we can't expect them
				   // to be automatically set to what the test harness expects. So just copy from
				   // the source.
				   const FAGX_ShapeMaterialBulkProperties& SourceBulk =
					   Source->GetShapeMaterialBulkProperties();
				   const FAGX_ShapeMaterialBulkProperties& DestinationBulk =
					   Destination->GetShapeMaterialBulkProperties();
				   memcpy(
					   (void*) &DestinationBulk, &SourceBulk,
					   sizeof(FAGX_ShapeMaterialBulkProperties));
				   const FAGX_ShapeMaterialSurfaceProperties& SourceSurface =
					   Source->GetShapeMaterialSurfaceProperties();
				   const FAGX_ShapeMaterialSurfaceProperties& DestinationSurface =
					   Destination->GetShapeMaterialSurfaceProperties();
				   memcpy(
					   (void*) &DestinationSurface, &SourceSurface,
					   sizeof(FAGX_ShapeMaterialSurfaceProperties));
				   const FAGX_ShapeMaterialWireProperties& SourceWire =
					   Source->GetShapeMaterialWireProperties();
				   const FAGX_ShapeMaterialWireProperties& DestinationWire =
					   Destination->GetShapeMaterialWireProperties();
				   memcpy(
					   (void*) &DestinationWire, &SourceWire,
					   sizeof(FAGX_ShapeMaterialWireProperties));

				   AssertAllRealPropertiesHaveIncreasingValues(Destination, *this);
			   });
		});
}
