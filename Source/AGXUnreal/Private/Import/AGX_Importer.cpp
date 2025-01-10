// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_Importer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AnyConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Import/AGX_ImporterSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Import/AGXSimObjectsReader.h"
#include "Import/SimulationObjectCollection.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Vehicle/TrackBarrier.h"

namespace AGX_Importer_helpers
{
	EAGX_ImportResult& operator|=(EAGX_ImportResult& InOutLhs, EAGX_ImportResult InRhs)
	{
		uint8 Lhs = (uint8) InOutLhs;
		uint8 Rhs = (uint8) InRhs;
		uint8 result = Lhs | Rhs;
		InOutLhs = (EAGX_ImportResult) result;
		return InOutLhs;
	}

	EAGX_ImportResult operator&(EAGX_ImportResult InLhs, EAGX_ImportResult InRhs)
	{
		uint8 Lhs = (uint8) InLhs;
		uint8 Rhs = (uint8) InRhs;
		uint8 Result = Lhs & Rhs;
		return (EAGX_ImportResult) Result;
	}

	bool IsUnrecoverableError(EAGX_ImportResult Result)
	{
		return (uint8) (Result & EAGX_ImportResult::FatalError) != 0 ||
			   (uint8) (Result & EAGX_ImportResult::Invalid) != 0;
	}

	AActor* CreateActor(const FString& Name)
	{
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to create an Actor during import, got an invalid empty name."));
			return nullptr;
		}

		AActor* NewActor = NewObject<AActor>();
		if (!NewActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create new actor during import."));
			return nullptr;
		}

		NewActor->Rename(*Name);
		return NewActor;
	}

	bool CreateSimulationObjectCollection(
		const FString& FilePath, FSimulationObjectCollection& OutSimObjects)
	{
		if (!FAGXSimObjectsReader::ReadAGXArchive(FilePath, OutSimObjects))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to import file '%s'. Log category LogAGX in the "
					 "Output Log may contain more information."),
				*FilePath);
			return false;
		}

		return true;
	}

	FString GetModelName(const FString& FilePath)
	{
		FString Name = FAGX_ObjectUtilities::SanitizeObjectName(FPaths::GetBaseFilename(FilePath));
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Unable to generate a valid model name from file '%s'."),
				*FilePath);
			return "";
		}

		return Name;
	}

	void PostCreateComponent(UActorComponent& Component, AActor& Owner)
	{
		Component.SetFlags(RF_Transactional);
		Owner.AddInstanceComponent(&Component);
	}

	template <typename T>
	auto& GetComponentsMapFrom(FAGX_AGXToUeContext& Context)
	{
		if constexpr (std::is_same_v<T, UAGX_RigidBodyComponent>)
			return *Context.RigidBodies.Get();

		if constexpr (std::is_same_v<T, UAGX_SphereShapeComponent>)
			return *Context.Shapes.Get();

		// Unsupported types will yield compile errors.
	}

	UAGX_RigidBodyComponent* GetOwningRigidBody(
		const FShapeBarrier& Shape, const FAGX_AGXToUeContext& Context, const AActor& Actor)
	{
		FRigidBodyBarrier BodyBarrier = Shape.GetRigidBody();
		if (!BodyBarrier.HasNative())
			return nullptr;

		UAGX_RigidBodyComponent* Body = Context.RigidBodies->FindRef(BodyBarrier.GetGuid());
		AGX_CHECK(Body != nullptr);
		return Body;
	}
}

FAGX_Importer::FAGX_Importer()
{
	Context.RigidBodies = MakeUnique<decltype(FAGX_AGXToUeContext::RigidBodies)::ElementType>();
	Context.Shapes = MakeUnique<decltype(FAGX_AGXToUeContext::Shapes)::ElementType>();

	Context.MSThresholds = MakeUnique<decltype(FAGX_AGXToUeContext::MSThresholds)::ElementType>();
}

FAGX_ImportResult FAGX_Importer::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_Importer_helpers;

	const FString Name = GetModelName(Settings.FilePath);
	if (Name.IsEmpty())
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	AActor* Actor = CreateActor(Name);
	if (Actor == nullptr)
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	FSimulationObjectCollection SimObjects;
	if (!CreateSimulationObjectCollection(Settings.FilePath, SimObjects))
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	EAGX_ImportResult Result = AddComponents(SimObjects, Settings, *Actor);
	if (IsUnrecoverableError(Result))
		return FAGX_ImportResult(Result);

	return FAGX_ImportResult(Result, Actor, &Context);
}

const FAGX_AGXToUeContext& FAGX_Importer::GetContext() const
{
	return Context;
}

template <typename TComponent, typename TBarrier>
EAGX_ImportResult FAGX_Importer::AddComponent(
	const TBarrier& Barrier, USceneComponent* Parent, AActor& OutActor)
{
	AGX_CHECK(Barrier.HasNative());
	if (!Barrier.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddComponent called on given a barrier that does not have a "
				 "Native. The Component will not be created."));
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	const FGuid Guid = Barrier.GetGuid();
	auto& ProcessedComponents = AGX_Importer_helpers::GetComponentsMapFrom<TComponent>(Context);
	if (ProcessedComponents.FindRef(Guid) != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddComponent called on Component '%s' that has already been "
				 "added."),
			*Barrier.GetName());
		AGX_CHECK(false);
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	TComponent* Component = NewObject<TComponent>(&OutActor);
	const FString Name =
		FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(&OutActor, Barrier.GetName());
	Component->CopyFrom(Barrier, &Context);

	if (Parent != nullptr)
	{
		Component->AttachToComponent(
			Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	AGX_Importer_helpers::PostCreateComponent(*Component, OutActor);
	ProcessedComponents.Add(Guid, Component);
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddModelSourceComponent(
	const FAGX_ImporterSettings& Settings, AActor& Owner)
{
	const FString Name = "AGX_ModelSource";
	if (Context.ModelSourceComponent != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddModelSourceComponent called, but a ModelSourceComponent has "
				 "already been added."),
			*Name);
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	UAGX_ModelSourceComponent* Component = NewObject<UAGX_ModelSourceComponent>(&Owner);
	Component->Rename(*Name);
	Component->FilePath = Settings.FilePath;
	Component->bIgnoreDisabledTrimeshes = Settings.bIgnoreDisabledTrimeshes;
	AGX_Importer_helpers::PostCreateComponent(*Component, Owner);
	Context.ModelSourceComponent = Component;
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddComponents(
	const FSimulationObjectCollection& SimObjects, const FAGX_ImporterSettings& Settings,
	AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	EAGX_ImportResult Result = EAGX_ImportResult::Success;

	for (const auto& Body : SimObjects.GetRigidBodies())
		Result |= AddComponent<UAGX_RigidBodyComponent, FRigidBodyBarrier>(Body, nullptr, OutActor);

	for (const auto& Shape : SimObjects.GetSphereShapes())
	{
		UAGX_RigidBodyComponent* Parent = GetOwningRigidBody(Shape, Context, OutActor);
		Result |= AddComponent<UAGX_SphereShapeComponent, FShapeBarrier>(Shape, Parent, OutActor);
	}

	Result |= AddModelSourceComponent(Settings, OutActor);

	return Result;
}
