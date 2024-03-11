#include "Sensors/AGX_SensorEnvironment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "AGX_Simulation.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_SensorEnvironmentSpriteComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

namespace AGX_SensorEnvironment_helpers
{
	bool Add(AAGX_SensorEnvironment& Env, UStaticMeshComponent* Mesh)
	{
		if (Mesh == nullptr)
			return false;

		const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
		if (StaticMesh == nullptr)
			return false;

		const uint32 LodIndex = StaticMesh->GetNumLODs() - 1; // Use highest LOD (low res) for now.
		if (!StaticMesh->HasValidRenderData(/*bCheckLODForVerts*/ true, LodIndex))
			return false;

		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;

		FAGX_MeshWithTransform MeshWTransform(StaticMesh, Mesh->GetComponentTransform());
		const bool Result = AGX_MeshUtilities::GetStaticMeshCollisionData(
			MeshWTransform, Mesh->GetComponentTransform(), OutVertices, OutIndices, &LodIndex);

		if (!Result)
			return false;

		return Env.Add(Mesh, OutVertices, OutIndices);
	}

}

AAGX_SensorEnvironment::AAGX_SensorEnvironment()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<UAGX_SensorEnvironmentSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
}

void AAGX_SensorEnvironment::Step(double DeltaTime)
{
	if (!HasNative())
		return;

	if (bAutoAddObjects)
		StepAutoAddObjects(DeltaTime);
	else
		StepNoAutoAddObjects(DeltaTime);

	NativeBarrier.Step(DeltaTime);

	for (auto It = ActiveLidars.CreateIterator(); It; ++It)
	{
		It->Key->GetResultTest(); // Todo: dummy test, remove this.
	}
}

bool AAGX_SensorEnvironment::AddMesh(UStaticMeshComponent* Mesh)
{
	return AGX_SensorEnvironment_helpers::Add(*this, Mesh);
}

bool AAGX_SensorEnvironment::AddTerrain(AAGX_Terrain* Terrain)
{
	if (!HasNative() || Terrain == nullptr)
		return false;

	if (Terrain->bEnableTerrainPaging)
		NativeBarrier.Add(*Terrain->GetOrCreateNativeTerrainPager());
	else
		NativeBarrier.Add(*Terrain->GetOrCreateNative());

	return true;
}

bool AAGX_SensorEnvironment::Add(
	UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	if (Mesh == nullptr)
		return false;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Sensor Environment Add was called on '%s' that does not have a Native. "
				 "This function is only valid to call during Play."),
			*GetName());
		return false;
	}

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (StaticMeshes.Contains(Mesh))
		return false;

	FMeshEntityBarrierData& MeshEntity = StaticMeshes.Add(Mesh, FMeshEntityBarrierData());
	MeshEntity.Transform = Mesh->GetComponentTransform();
	MeshEntity.Mesh.AllocateNative(Vertices, Indices);
	MeshEntity.Entity.AllocateNative(MeshEntity.Mesh);
	MeshEntity.Entity.SetTransform(MeshEntity.Transform);
	return true;
}

bool AAGX_SensorEnvironment::Remove(UStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr || !StaticMeshes.Contains(Mesh))
		return false;

	StaticMeshes.Remove(Mesh);
	return true;
}

bool AAGX_SensorEnvironment::HasNative() const
{
	return NativeBarrier.HasNative();
}

FSensorEnvironmentBarrier* AAGX_SensorEnvironment::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FSensorEnvironmentBarrier* AAGX_SensorEnvironment::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool AAGX_SensorEnvironment::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, LidarSensors),
			GET_MEMBER_NAME_CHECKED(ThisClass, bAutoStep),
			GET_MEMBER_NAME_CHECKED(ThisClass, bAutoAddObjects)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}

void AAGX_SensorEnvironment::BeginPlay()
{
	Super::BeginPlay();
	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_SensorEnvironment '%s' has a Native assigned at the start of BeginPlay which "
				 "is unexpected. Correct behavior of the SensorEnvironment cannot be guaranteed."),
			*GetName());
		return;
	}

	NativeBarrier.AllocateNative();
	check(NativeBarrier.HasNative());

	RegisterLidars();

	if (bAutoStep)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			PostStepForwardHandle =
				FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
					.AddLambda([this](double) { AutoStep(); });
		}
	}
}

void AAGX_SensorEnvironment::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (Reason != EEndPlayReason::EndPlayInEditor && Reason != EEndPlayReason::Quit &&
		Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.Remove(PostStepForwardHandle);
		}
	}
}

void AAGX_SensorEnvironment::RegisterLidars()
{
	check(HasNative());
	TSet<UPrimitiveComponent*> OverlappingComponents;

	for (FAGX_LidarSensorReference& LidarRef : LidarSensors)
	{
		if (UAGX_LidarSensorComponent* Lidar = LidarRef.GetLidarComponent())
		{
			NativeBarrier.Add(*Lidar->GetOrCreateNative());

			// Associate each Lidar with a USphereComponent used to detect objects in the world to
			// give to AGX Dynamics during Play.
			USphereComponent* CollSph = nullptr;
			if (bAutoAddObjects)
			{
				CollSph = NewObject<USphereComponent>(this);
				CollSph->RegisterComponent();
				CollSph->SetWorldTransform(Lidar->GetComponentTransform());
				CollSph->SetSphereRadius(Lidar->Range, /*bUpdateOverlaps*/ true);

				// Fetch all currently overlapping Components.
				TSet<UPrimitiveComponent*> OverlComp;
				CollSph->GetOverlappingComponents(OverlComp);
				OverlappingComponents.Append(OverlComp);
			}

			ActiveLidars.Add(Lidar, CollSph);
		}
	}

	if (bAutoAddObjects)
	{
		// Add overlapping Components so they become visible to the sensors handled by this
		// Environment.
		for (UPrimitiveComponent* Overlapping : OverlappingComponents)
		{
			if (UStaticMeshComponent* Sm = Cast<UStaticMeshComponent>(Overlapping))
				AddMesh(Sm);
		}

		// Add Terrains.
		const ULevel* Level = GetWorld()->GetCurrentLevel();

		// Unsafe to use range based loop here since Actors array may grow if for example the
		// AGX_Stepper is added during this.
		for (int32 i = 0; i < Level->Actors.Num(); i++)
		{
			if (AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(Level->Actors[i]))
			{
				AddTerrain(Terrain);
			}
		}
	}
}

void AAGX_SensorEnvironment::AutoStep()
{
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		Step(Simulation->TimeStep);
	}
}

void AAGX_SensorEnvironment::StepNoAutoAddObjects(double DeltaTime)
{
	for (auto It = ActiveLidars.CreateIterator(); It; ++It)
	{
		It->Key->Step();
	}
}

void AAGX_SensorEnvironment::StepAutoAddObjects(double DeltaTime)
{
	// Todo: update collision spheres and add/remove objects.
	for (auto It = ActiveLidars.CreateIterator(); It; ++It)
	{
		It->Key->Step();
	}
}
