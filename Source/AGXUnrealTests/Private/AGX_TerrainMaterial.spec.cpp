// Copyright 2024, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Materials/AGX_TerrainMaterial.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

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
				Callback(PropertyMemory);
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
				Callback(PropertyMemory);
			}
			else if (FStructProperty* NestedStructProperty = CastField<FStructProperty>(Property))
			{
				VisitProperties(NestedStructProperty, PropertyMemory, CPPType, Callback);
			}
		}
	}

	double SetAllRealPropertiesToIncreasingValues(
		double NextValue, FStructProperty* StructProperty, void* StructMemory)
	{
		// Loop over AdhesionOverlapFactor, Cohesion, Density, etc.
		UScriptStruct* Struct = StructProperty->Struct;
		for (TFieldIterator<FProperty> PropertyIt(Struct); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			UE_LOG(LogAGX, Warning, TEXT("  Found property of type %s"), *Property->GetCPPType());
			if (FStructProperty* NestedStructProperty = CastField<FStructProperty>(Property))
			{
				UE_LOG(
					LogAGX, Warning, TEXT("  Found nested struct of type %s."),
					*NestedStructProperty->Struct->GetName());
				void* PropertyMemory = Property->ContainerPtrToValuePtr<void>(StructMemory);
				if (NestedStructProperty->Struct == FAGX_Real::StaticStruct())
				{
					UE_LOG(
						LogAGX, Warning, TEXT("    Found FAGX_Real struct, writing %f."),
						NextValue);
					FAGX_Real* Real = static_cast<FAGX_Real*>(PropertyMemory);
					Real->Value = NextValue;
					NextValue += 1.0;
				}
				else
				{
					UE_LOG(LogAGX, Warning, TEXT("Is a struct, recursion."));
					NextValue = SetAllRealPropertiesToIncreasingValues(
						NextValue, NestedStructProperty, PropertyMemory);
				}
			}
		}

		return NextValue;
	}

	void SetAllRealPropertiesToIncreasingValues(UObject* Object)
	{
		auto Callback = [NextValue = 0.0](void* Memory) mutable
		{
			FAGX_Real* Real = static_cast<FAGX_Real*>(Memory);
			UE_LOG(LogAGX, Warning, TEXT("  Overwriting %f with %f."), Real->Value, NextValue);
			Real->Value = NextValue;
			NextValue = NextValue + 1.0;
		};

		VisitProperties(Object, TEXT("FAGX_Real"), Callback);
	}

	void AssertAllRealPropertiesHaveIncreasingValues(UObject* Object)
	{
		auto Callback = [NextValue = 0.0](void* Memory) mutable
		{
			FAGX_Real* Real = static_cast<FAGX_Real*>(Memory);
			UE_LOG(LogAGX, Warning, TEXT("  Expecting %f, found %f."), NextValue, Real->Value);
			if (NextValue != Real->Value)
			{
				UE_LOG(LogAGX, Warning, TEXT("     MISMATCH!"));
			}
			NextValue = NextValue + 1.0;
		};

		VisitProperties(Object, TEXT("FAGX_Real"), Callback);
	}
}

void FAGX_TerrainMaterialSpec::Define()
{
	UE_LOG(LogAGX, Warning, TEXT("Discovering FAGX_TerrainMaterialSpec"));
	Describe(
		"When copying properties from one Terrain Material to another",
		[this]()
		{
			It("should copy all properties",
			   [this]()
			   {
				   TObjectPtr<UAGX_TerrainMaterial> Source = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Source Terrain Material"));

				   AGX_TerrainMaterialSpec_helpers::SetAllRealPropertiesToIncreasingValues(Source);
				   AGX_TerrainMaterialSpec_helpers::AssertAllRealPropertiesHaveIncreasingValues(
					   Source);

			   	TObjectPtr<UAGX_TerrainMaterial> Destination =  NewObject<UAGX_TerrainMaterial>(GetTransientPackage(), TEXT("Destination Terrain Material"));
			   	Destination->CopyTerrainMaterialProperties(Source);
			   	AGX_TerrainMaterialSpec_helpers::AssertAllRealPropertiesHaveIncreasingValues(Destination);

#if 0
				   TObjectPtr<UAGX_TerrainMaterial> Destination = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Destination Terrain Materia."));

				   UE_LOG(LogAGX, Warning, TEXT("Source: %p"), Source);
				   UE_LOG(LogAGX, Warning, TEXT("TerrainBulk:  %p"), &Source->TerrainBulk);
				   UE_LOG(
					   LogAGX, Warning, TEXT("AdhesionOverlapFactor: %p"),
					   &Source->TerrainBulk.AdhesionOverlapFactor);
				   UE_LOG(
					   LogAGX, Warning, TEXT("Value: %p"),
					   &Source->TerrainBulk.AdhesionOverlapFactor.Value);
				   UE_LOG(
					   LogAGX, Warning, TEXT("Value: %f"),
					   Source->TerrainBulk.AdhesionOverlapFactor.Value);

				   // Loop over TerrainBulk, TerrainCompaction, etc.
				   UClass* Class = UAGX_TerrainMaterial::StaticClass();
				   for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
				   {
					   FProperty* TerrainMaterialProperty = *PropIt;
					   UE_LOG(
						   LogAGX, Warning, TEXT("Got property named '%s' from owner class '%s'."),
						   *TerrainMaterialProperty->GetName(),
						   *TerrainMaterialProperty->GetOwnerClass()->GetName());

					   FStructProperty* TerrainMaterialStructProperty =
						   CastField<FStructProperty>(TerrainMaterialProperty);
					   void* StructPtr =
						   TerrainMaterialStructProperty->ContainerPtrToValuePtr<void>(Source);
					   UE_LOG(LogAGX, Warning, TEXT("  Struct: %p"), StructPtr);
					   UE_LOG(LogAGX, Warning, TEXT("Looping over struct members:"));

#if 1
					   // Loop over AdhesionOverlapFactor, Cohesion, Density, etc.
					   TObjectPtr<UScriptStruct> Struct = TerrainMaterialStructProperty->Struct;
					   for (TFieldIterator<FProperty> StructPropIt(Struct); StructPropIt;
							++StructPropIt)
					   {
						   FProperty* StructMemberProperty = *StructPropIt;
						   UE_LOG(
							   LogAGX, Warning, TEXT("  Struct member %s"),
							   *StructMemberProperty->GetName());
						   void* RealStructPtr =
							   StructPropIt->ContainerPtrToValuePtr<void>(StructPtr);
						   UE_LOG(LogAGX, Warning, TEXT("  RealStructPtr: %p"), RealStructPtr);
#if 1
						   FAGX_Real* RealValue = (FAGX_Real*) RealStructPtr;
						   UE_LOG(LogAGX, Warning, TEXT("  RealStructValue: %f"), RealValue->Value);
						   StructMemberProperty->HasAnyPropertyFlags(CPF_Deprecated);

#else
						   FStructProperty* RealProperty =
							   CastField<FStructProperty>(StructMemberProperty);
						   for (TFieldIterator<FProperty> RealPropIt(RealProperty->Struct);
								RealPropIt; ++RealPropIt)
						   {
							   FProperty* RealInnerProperty = *RealPropIt;
							   UE_LOG(
								   LogAGX, Warning, TEXT("    Real property: %s"),
								   *RealInnerProperty->GetName());
							   if (FDoubleProperty* DoubleProperty =
									   CastField<FDoubleProperty>(RealInnerProperty))
							   {
								   UE_LOG(LogAGX, Warning, TEXT("Is a Double property."));
								   void* StructMemory =
									   RealInnerProperty->ContainerPtrToValuePtr<void>(Source);
								   double DoubleValue = DoubleProperty->GetPropertyValue(
									   RealProperty->ContainerPtrToValuePtr<void>(StructMemory));
								   UE_LOG(LogTemp, Log, TEXT("    Double value: %f"), DoubleValue);
							   }
							   else
							   {
								   UE_LOG(
									   LogAGX, Warning, TEXT("    Property %s has unknown type."),
									   *RealInnerProperty->GetName());
							   }
						   }
#endif
					   }
#else
				for (FField* StructMember = Struct->ChildProperties; StructMember != nullptr;
					 StructMember = StructMember->Next)
				{
					UE_LOG(
						LogAGX, Warning, TEXT("  Got sub-property named '%s' of type '%s'."),
						*StructMember->GetName(), *StructMember->GetClass()->GetName());

					const FProperty* ChildProperty = nullptr;
					const void* ChildData = nullptr;
					TerrainMaterialStructProperty->FindInnerPropertyInstance(
						StructMember->GetFName(), StructPtr, ChildProperty, ChildData);
					const FAGX_Real* AsReal = reinterpret_cast<const FAGX_Real*>(ChildData);
					UE_LOG(LogAGX, Warning, TEXT("  Has value: %d"), AsReal->Value);
				}
				UE_LOG(LogAGX, Warning, TEXT("No more struct members."))

				// TestEqual(
				//  TEXT("Class"), reinterpret_cast<intptr_t>(Class),
				//  reinterpret_cast<intptr_t>(Property->GetClass()));
#endif
				   };
#endif
			   });
		});
}
