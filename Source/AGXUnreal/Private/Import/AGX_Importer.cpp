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
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
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

	void WriteImportTag(UActorComponent& Component, const FGuid& SessionGuid)
	{
		Component.ComponentTags.Empty();
		Component.ComponentTags.Add(*SessionGuid.ToString());
	}

	void PostCreateComponent(UActorComponent& Component, AActor& Owner, const FGuid& SessionGuid)
	{
		WriteImportTag(Component, SessionGuid);
		Component.SetFlags(RF_Transactional);
		Owner.AddInstanceComponent(&Component);
	}

	AActor* CreateActor(const FString& Name, const FGuid& SessionGuid)
	{
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to create an Actor during import, got an invalid empty name."));
			return nullptr;
		}

		AActor* NewActor = NewObject<AActor>(GetTransientPackage(), *Name);
		if (!NewActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create new actor during import."));
			return nullptr;
		}

		auto Root = NewObject<USceneComponent>(NewActor, FName(TEXT("DefaultSceneRoot")));
		PostCreateComponent(*Root, *NewActor, SessionGuid);
		NewActor->SetRootComponent(Root);

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

	template <typename T>
	auto& GetComponentsMapFrom(FAGX_ImportContext& Context)
	{
		if constexpr (std::is_same_v<T, UAGX_RigidBodyComponent>)
			return *Context.RigidBodies.Get();

		if constexpr (std::is_base_of_v<UAGX_ShapeComponent, T>)
			return *Context.Shapes.Get();

		// Unsupported types will yield compile errors.
	}

	USceneComponent* GetOwningRigidBodyOrRoot(
		const FShapeBarrier& Shape, const FAGX_ImportContext& Context, const AActor& Actor)
	{
		FRigidBodyBarrier BodyBarrier = Shape.GetRigidBody();
		if (!BodyBarrier.HasNative())
			return Actor.GetRootComponent();

		UAGX_RigidBodyComponent* Body = Context.RigidBodies->FindRef(BodyBarrier.GetGuid());
		check(Body != nullptr);
		return Body;
	}
}

FAGX_Importer::FAGX_Importer()
{
	Context.RigidBodies = MakeUnique<decltype(FAGX_ImportContext::RigidBodies)::ElementType>();
	Context.Shapes = MakeUnique<decltype(FAGX_ImportContext::Shapes)::ElementType>();
	Context.RenderStaticMeshCom = MakeUnique<TMap<FGuid, UStaticMeshComponent*>>();
	Context.RenderMaterials = MakeUnique<TMap<FGuid, UMaterialInterface*>>();
	Context.RenderStaticMeshes = MakeUnique<TMap<FGuid, UStaticMesh*>>();
	Context.CollisionStaticMeshes = MakeUnique<TMap<FGuid, UStaticMesh*>>();
	Context.MSThresholds = MakeUnique<decltype(FAGX_ImportContext::MSThresholds)::ElementType>();
}

FAGX_ImportResult FAGX_Importer::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_Importer_helpers;

	Context.Settings = &Settings;
	Context.SessionGuid = FGuid::NewGuid();

	const FString Name = GetModelName(Settings.FilePath);
	if (Name.IsEmpty())
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	AActor* Actor = CreateActor(Name, Context.SessionGuid);
	if (Actor == nullptr)
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	FSimulationObjectCollection SimObjects;
	if (!CreateSimulationObjectCollection(Settings.FilePath, SimObjects))
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	EAGX_ImportResult Result = AddComponents(SimObjects, *Actor);
	if (IsUnrecoverableError(Result))
		return FAGX_ImportResult(Result);

	return FAGX_ImportResult(Result, Actor, &Context);
}

const FAGX_ImportContext& FAGX_Importer::GetContext() const
{
	return Context;
}

template <typename TComponent, typename TBarrier>
EAGX_ImportResult FAGX_Importer::AddComponent(
	const TBarrier& Barrier, USceneComponent& Parent, AActor& OutActor)
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
	Component->AttachToComponent(&Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	AGX_Importer_helpers::PostCreateComponent(*Component, OutActor, Context.SessionGuid);
	ProcessedComponents.Add(Guid, Component);
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddModelSourceComponent(AActor& Owner)
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
	Component->FilePath = Context.Settings->FilePath;
	Component->bIgnoreDisabledTrimeshes = Context.Settings->bIgnoreDisabledTrimeshes;
	AGX_Importer_helpers::PostCreateComponent(*Component, Owner, Context.SessionGuid);
	Context.ModelSourceComponent = Component;
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddComponents(
	const FSimulationObjectCollection& SimObjects, AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	EAGX_ImportResult Result = EAGX_ImportResult::Success;
	USceneComponent* Root = OutActor.GetRootComponent();
	check(Root != nullptr);

	for (const auto& Body : SimObjects.GetRigidBodies())
		Result |= AddComponent<UAGX_RigidBodyComponent, FRigidBodyBarrier>(Body, *Root, OutActor);

	for (const auto& Shape : SimObjects.GetBoxShapes())
		AddShape<UAGX_BoxShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetCapsuleShapes())
		AddShape<UAGX_CapsuleShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetCylinderShapes())
		AddShape<UAGX_CylinderShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetSphereShapes())
		AddShape<UAGX_SphereShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetTrimeshShapes())
		AddTrimeshShape(Shape, OutActor);

	Result |= AddModelSourceComponent(OutActor);

	return Result;
}

template <typename TShapeComponent>
EAGX_ImportResult FAGX_Importer::AddShape(const FShapeBarrier& Shape, AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	auto Parent = GetOwningRigidBodyOrRoot(Shape, Context, OutActor);
	return AddComponent<TShapeComponent, FShapeBarrier>(Shape, *Parent, OutActor);
}

EAGX_ImportResult FAGX_Importer::AddTrimeshShape(const FShapeBarrier& Shape, AActor& OutActor)
{
	auto Result = AddShape<UAGX_TrimeshShapeComponent>(Shape, OutActor);

	if (Context.Settings->bIgnoreDisabledTrimeshes && !Shape.GetEnableCollisions())
	{
		// We don't want to import the Trimesh but we do want to import the Render Data.
		// For simplicity, the have imported the Trimesh as usual above, but will not remove the
		// Trimesh Component. The Trimesh Component will know to not create a collision Mesh, so we
		// don't need to consider that.
		auto Trimesh = Context.Shapes->FindRef(Shape.GetShapeGuid());
		AGX_CHECK(Trimesh != nullptr);
		if (Trimesh == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not find Trimesh '%s' that should have been imported."),
				*Shape.GetName());
			return EAGX_ImportResult::RecoverableErrorsOccured;
		}

		auto Res = FAGX_ObjectUtilities::RemoveComponentAndPromoteChildren(Trimesh, &OutActor);
		AGX_CHECK(Res);
	}

	return EAGX_ImportResult::Success;
}
