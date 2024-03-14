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

	void UpdateCollisionSphere(const UAGX_LidarSensorComponent* Lidar, USphereComponent* Sphere)
	{
		if (Lidar == nullptr || Sphere == nullptr)
			return;

		if (!FMath::IsNearlyEqual(
				Sphere->GetUnscaledSphereRadius(), static_cast<float>(Lidar->Range)))
		{
			Sphere->SetSphereRadius(Lidar->Range, /*bUpdateOverlaps*/ true);
		}

		Sphere->SetWorldLocation(Lidar->GetComponentLocation());
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

	for (auto It = TrackedLidars.CreateIterator(); It; ++It)
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
		return NativeBarrier.Add(*Terrain->GetOrCreateNativeTerrainPager());
	else
		return NativeBarrier.Add(*Terrain->GetOrCreateNative());
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

	if (TrackedStaticMeshes.Contains(Mesh))
		return false;

	FAGX_MeshEntityData& MeshEntity = TrackedStaticMeshes.Add(Mesh, FAGX_MeshEntityData());
	MeshEntity.Mesh.AllocateNative(Vertices, Indices);
	MeshEntity.Entity.AllocateNative(MeshEntity.Mesh);
	MeshEntity.SetTransform(Mesh->GetComponentTransform());
	return true;
}

bool AAGX_SensorEnvironment::RemoveMesh(UStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr || !TrackedStaticMeshes.Contains(Mesh))
		return false;

	TrackedStaticMeshes.Remove(Mesh);
	return true;
}

bool AAGX_SensorEnvironment::RemoveTerrain(AAGX_Terrain* Terrain)
{
	if (!HasNative() || Terrain == nullptr || !Terrain->HasNative())
		return false;

	if (Terrain->bEnableTerrainPaging)
		return NativeBarrier.Remove(*Terrain->GetNativeTerrainPager());
	else
		return NativeBarrier.Remove(*Terrain->GetOrCreateNative());
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
				AGX_SensorEnvironment_helpers::UpdateCollisionSphere(Lidar, CollSph);

				// Fetch all currently overlapping Components.
				TSet<UPrimitiveComponent*> OverlComp;
				CollSph->GetOverlappingComponents(OverlComp);
				CollSph->OnComponentBeginOverlap.AddDynamic(
					this, &AAGX_SensorEnvironment::OnLidarBeginOverlapComponent);
				CollSph->OnComponentEndOverlap.AddDynamic(
					this, &AAGX_SensorEnvironment::OnLidarEndOverlapComponent);
				OverlappingComponents.Append(OverlComp);
			}

			TrackedLidars.Add(Lidar, CollSph);
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
	check(!bAutoAddObjects);
	UpdateTrackedLidars();
	StepTrackedLidars();
}

void AAGX_SensorEnvironment::StepAutoAddObjects(double DeltaTime)
{
	check(bAutoAddObjects);
	UpdateTrackedLidars(); // Will likely trigger Component overlap events.
	UpdateTrackedStaticMeshes();
	StepTrackedLidars();
}

void AAGX_SensorEnvironment::UpdateTrackedLidars()
{
	// Update Collision Spheres and remove any destroyed Lidars.
	// Notice that overlap events will likely be triggered when updating the collision spheres radii
	// and transform.
	for (auto It = TrackedLidars.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key))
		{
			It.RemoveCurrent();
			continue;
		}

		if (bAutoAddObjects)
			AGX_SensorEnvironment_helpers::UpdateCollisionSphere(It->Key, It->Value);
	}
}

void AAGX_SensorEnvironment::UpdateTrackedStaticMeshes()
{
	check(bAutoAddObjects);

	// Update tracked static meshes and remove any invalid ones.
	for (auto It = TrackedStaticMeshes.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key))
		{
			It.RemoveCurrent();
			continue;
		}

		const FTransform& CompTransform = It->Key->GetComponentTransform();
		if (CompTransform.Equals(It->Value.Transform))
			continue;

		It->Value.SetTransform(CompTransform);
	}
}

void AAGX_SensorEnvironment::StepTrackedLidars() const
{
	for (auto It = TrackedLidars.CreateConstIterator(); It; ++It)
	{
		It->Key->Step();
	}
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapComponent(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherComp))
		return;

	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(OtherComp);
	if (Mesh == nullptr)
		return;

	FAGX_MeshEntityData* MeshEntityData = TrackedStaticMeshes.Find(Mesh);
	if (MeshEntityData != nullptr)
	{
		MeshEntityData->RefCount++;
		return;
	}

	AddMesh(Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapComponent(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsValid(OtherComp))
		return;

	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(OtherComp);
	if (Mesh == nullptr)
		return;

	FAGX_MeshEntityData* MeshEntityData = TrackedStaticMeshes.Find(Mesh);
	if (MeshEntityData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX_SensorEnvironment '%s' failed to track Static Mesh Component '%s'"),
			*Mesh->GetName());
		return;
	}

	AGX_CHECK(MeshEntityData->RefCount > 0);
	MeshEntityData->RefCount--;
	if (MeshEntityData->RefCount == 0)
		TrackedStaticMeshes.Remove(Mesh);
}
