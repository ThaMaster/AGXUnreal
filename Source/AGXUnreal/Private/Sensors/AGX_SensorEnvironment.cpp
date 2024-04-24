#include "Sensors/AGX_SensorEnvironment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "AGX_Simulation.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_SensorEnvironmentSpriteComponent.h"
#include "AGX_SimpleMeshComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

namespace AGX_SensorEnvironment_helpers
{
	bool GetVerticesIndices(
		UStaticMeshComponent* Mesh, TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices)
	{
		if (Mesh == nullptr)
			return false;

		const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
		if (StaticMesh == nullptr)
			return false;

		const uint32 LodIndex = StaticMesh->GetNumLODs() - 1; // Use highest LOD (low res) for now.
		if (!StaticMesh->HasValidRenderData(/*bCheckLODForVerts*/ true, LodIndex))
			return false;

		FAGX_MeshWithTransform MeshWTransform(StaticMesh, Mesh->GetComponentTransform());
		return AGX_MeshUtilities::GetStaticMeshCollisionData(
			MeshWTransform, Mesh->GetComponentTransform(), OutVertices, OutIndices, &LodIndex);
	}

	bool GetVerticesIndices(
		UAGX_SimpleMeshComponent* Mesh, TArray<FVector>& OutVertices,
		TArray<FTriIndices>& OutIndices)
	{
		if (Mesh == nullptr)
			return false;

		const FAGX_SimpleMeshData* MeshData = Mesh->GetMeshData();
		if (MeshData == nullptr)
			return false;

		OutVertices.Reserve(MeshData->Vertices.Num());
		for (const FVector3f& V : MeshData->Vertices)
		{
			OutVertices.Add(
				{static_cast<double>(V.X), static_cast<double>(V.Y), static_cast<double>(V.Z)});
		}

		OutIndices.Reserve(MeshData->Indices.Num() / 3);
		for (int32 I = 3; I < MeshData->Indices.Num(); I += 3)
		{
			FTriIndices TriInd;
			TriInd.v0 = static_cast<int32>(MeshData->Indices[I - 2]);
			TriInd.v1 = static_cast<int32>(MeshData->Indices[I - 1]);
			TriInd.v2 = static_cast<int32>(MeshData->Indices[I]);
			OutIndices.Add(TriInd);
		}

		return true;
	}

	void UpdateCollisionSphere(const UAGX_LidarSensorComponent* Lidar, USphereComponent* Sphere)
	{
		if (Lidar == nullptr || Sphere == nullptr)
			return;

		Sphere->SetWorldLocation(Lidar->GetComponentLocation());

		if (!FMath::IsNearlyEqual(
				Sphere->GetUnscaledSphereRadius(), static_cast<float>(Lidar->Range.Max)))
		{
			Sphere->SetSphereRadius(Lidar->Range.Max, /*bUpdateOverlaps*/ true);
		}
	}

	template <typename MeshesType>
	void UpdateTrackedMeshes(MeshesType& MeshesMap)
	{
		// Update tracked static meshes and remove any invalid ones.
		for (auto It = MeshesMap.CreateIterator(); It; ++It)
		{
			if (!IsValid(It->Key.Get()))
			{
				It.RemoveCurrent();
				continue;
			}

			const FTransform& CompTransform = It->Key->GetComponentTransform();
			if (CompTransform.Equals(It->Value.EntityData.Transform))
				continue;

			It->Value.EntityData.SetTransform(CompTransform);
		}
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
}

bool AAGX_SensorEnvironment::AddMesh(UStaticMeshComponent* Mesh)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Sensor Environment AddMesh was called on '%s' that does not have a Native. "
				 "This function is only valid to call during Play."),
			*GetName());
		return false;
	}

	TArray<FVector> OutVerts;
	TArray<FTriIndices> OutInds;
	const bool Res = AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds);
	if (Res)
		AddMesh(Mesh, OutVerts, OutInds);

	return Res;
}

bool AAGX_SensorEnvironment::AddAGXMesh(UAGX_SimpleMeshComponent* Mesh)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Sensor Environment AddAGXMesh was called on '%s' that does not have a Native. "
				 "This function is only valid to call during Play."),
			*GetName());
		return false;
	}

	TArray<FVector> OutVerts;
	TArray<FTriIndices> OutInds;
	const bool Res = AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds);
	if (Res)
		AddMesh(Mesh, OutVerts, OutInds);

	return Res;
}

bool AAGX_SensorEnvironment::AddInstancedMesh(UInstancedStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr)
		return false;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Sensor Environment AddInstancedMesh was called on '%s' that does not have "
				 "a Native. This function is only valid to call during Play."),
			*GetName());
		return false;
	}

	if (!TrackedInstancedMeshes.Contains(Mesh))
	{
		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;
		if (AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVertices, OutIndices))
		{
			AddInstancedMesh(Mesh, OutVertices, OutIndices);
		}
		else
		{
			return false;
		}
	}

	const int32 InstanceCnt = Mesh->GetInstanceCount();
	for (int32 i = 0; i < InstanceCnt; i++)
	{
		AddInstancedMeshInstance_Internal(Mesh, i);
	}

	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMeshInstance(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	if (Mesh == nullptr || !Mesh->IsValidInstance(Index))
		return false;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Sensor Environment AddInstancedMeshInstance was called on '%s' that does not have "
				"a Native. This function is only valid to call during Play."),
			*GetName());
		return false;
	}

	if (!TrackedInstancedMeshes.Contains(Mesh))
	{
		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;
		if (AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVertices, OutIndices))
			AddInstancedMesh(Mesh, OutVertices, OutIndices);
		else
			return false;
	}

	return AddInstancedMeshInstance_Internal(Mesh, Index);
}

bool AAGX_SensorEnvironment::AddMesh(
	UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedMeshes.Contains(Mesh))
		return false;

	FAGX_MeshEntityData& MeshEntity = TrackedMeshes.Add(Mesh, FAGX_MeshEntityData());
	MeshEntity.Mesh.AllocateNative(Vertices, Indices);
	MeshEntity.EntityData.Entity.AllocateNative(MeshEntity.Mesh);
	MeshEntity.EntityData.SetTransform(Mesh->GetComponentTransform());
	return true;
}

bool AAGX_SensorEnvironment::AddMesh(
	UAGX_SimpleMeshComponent* Mesh, const TArray<FVector>& Vertices,
	const TArray<FTriIndices>& Indices)
{
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedAGXMeshes.Contains(Mesh))
		return false;

	FAGX_MeshEntityData& MeshEntity = TrackedAGXMeshes.Add(Mesh, FAGX_MeshEntityData());
	MeshEntity.Mesh.AllocateNative(Vertices, Indices);
	MeshEntity.EntityData.Entity.AllocateNative(MeshEntity.Mesh);
	MeshEntity.EntityData.SetTransform(Mesh->GetComponentTransform());
	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMesh(
	UInstancedStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
	const TArray<FTriIndices>& Indices)
{
	AGX_CHECK(HasNative());
	if (Mesh == nullptr || Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedInstancedMeshes.Contains(Mesh))
		return false;

	auto& InstancedMeshEntity = TrackedInstancedMeshes.Add(Mesh, FAGX_InstancedMeshEntityData());
	InstancedMeshEntity.Mesh.AllocateNative(Vertices, Indices);
	AGX_CHECK(InstancedMeshEntity.Mesh.HasNative());
	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMeshInstance_Internal(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	AGX_CHECK(HasNative());
	AGX_CHECK(Mesh != nullptr);
	AGX_CHECK(Mesh->IsValidInstance(Index));

	FAGX_InstancedMeshEntityData* InstancedMeshEntity = TrackedInstancedMeshes.Find(Mesh);

	// This function should only be called for known Instanced Static Mesh Components.
	AGX_CHECK(InstancedMeshEntity != nullptr);
	if (InstancedMeshEntity == nullptr)
		return false;

	if (InstancedMeshEntity->EntitiesData.Contains(Index))
		return false; // We already track this instance.

	FAGX_EntityData& EntityData = InstancedMeshEntity->EntitiesData.Add(Index, FAGX_EntityData());
	EntityData.Entity.AllocateNative(InstancedMeshEntity->Mesh);
	AGX_CHECK(EntityData.Entity.HasNative());
	FTransform InstanceTrans;
	Mesh->GetInstanceTransform(Index, InstanceTrans, true);
	EntityData.SetTransform(InstanceTrans);
	return true;
}

bool AAGX_SensorEnvironment::AddTerrain(AAGX_Terrain* Terrain)
{
	if (!HasNative() || Terrain == nullptr)
		return false;

	if (Terrain->bEnableTerrainPaging)
	{
		FTerrainPagerBarrier* PagerBarrier = Terrain->GetOrCreateNativeTerrainPager();
		if (PagerBarrier == nullptr)
			return false;

		return NativeBarrier.Add(*PagerBarrier);
	}
	else
	{
		FTerrainBarrier* TerrainBarrier = Terrain->GetOrCreateNative();
		if (TerrainBarrier == nullptr)
			return false;

		return NativeBarrier.Add(*TerrainBarrier);
	}
}

bool AAGX_SensorEnvironment::RemoveMesh(UStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr || !TrackedMeshes.Contains(Mesh))
		return false;

	TrackedMeshes.Remove(Mesh);
	return true;
}

bool AAGX_SensorEnvironment::RemoveInstancedMesh(UInstancedStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr || !TrackedInstancedMeshes.Contains(Mesh))
		return false;

	TrackedInstancedMeshes.Remove(Mesh);
	return true;
}

bool AAGX_SensorEnvironment::RemoveInstancedMeshInstance(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	if (Mesh == nullptr)
		return false;

	auto InstancedMeshData = TrackedInstancedMeshes.Find(Mesh);
	if (InstancedMeshData == nullptr)
		return false;

	if (!InstancedMeshData->EntitiesData.Contains(Index))
		return false;

	InstancedMeshData->EntitiesData.Remove(Index);
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

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this);
	if (Sim == nullptr || !Sim->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_SensorEnvironment '%s' was unable to get a UAGX_Simulation with Native "
				 "in begin play. Correct behavior of the SensorEnvironment cannot be guaranteed."),
			*GetName());
		return;
	}

	NativeBarrier.AllocateNative(*Sim->GetNative());
	check(NativeBarrier.HasNative());

	RegisterLidars();

	PostStepForwardHandle =
		FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Sim).AddLambda(
			[this](double) { AutoStep(); });
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

	TrackedLidars.Empty();
	TrackedMeshes.Empty();
	TrackedInstancedMeshes.Empty();
	TrackedAGXMeshes.Empty();
	if (HasNative())
		NativeBarrier.ReleaseNative();
}

void AAGX_SensorEnvironment::RegisterLidars()
{
	AGX_CHECK(HasNative());
	TSet<UPrimitiveComponent*> OverlappingComponents;

	for (FAGX_LidarSensorReference& LidarRef : LidarSensors)
	{
		if (UAGX_LidarSensorComponent* Lidar = LidarRef.GetLidarComponent())
		{
			FLidarBarrier* Barrier = Lidar->GetOrCreateNative();
			if (Barrier == nullptr)
				continue;

			NativeBarrier.Add(*Barrier);

			// Associate each Lidar with a USphereComponent used to detect objects in the world to
			// give to AGX Dynamics during Play.
			USphereComponent* CollSph = nullptr;
			if (bAutoAddObjects)
			{
				CollSph = NewObject<USphereComponent>(this);
				CollSph->OnComponentBeginOverlap.AddDynamic(
					this, &AAGX_SensorEnvironment::OnLidarBeginOverlapComponent);
				CollSph->OnComponentEndOverlap.AddDynamic(
					this, &AAGX_SensorEnvironment::OnLidarEndOverlapComponent);

				// Ensure we don't miss overlap events by setting radius zero now. All collision
				// Collision spheres are updated in Step(), and the overlap events will be triggered
				// for any object within that radius.
				CollSph->SetSphereRadius(0.f, false);
				CollSph->RegisterComponent();
			}

			TrackedLidars.Add(Lidar, CollSph);
		}
	}

	if (bAutoAddObjects)
	{
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
	UpdateTrackedMeshes();
	UpdateTrackedAGXMeshes();
	StepTrackedLidars();
}

void AAGX_SensorEnvironment::UpdateTrackedLidars()
{
	// Update Collision Spheres and remove any destroyed Lidars.
	// Notice that overlap events will likely be triggered when updating the collision spheres radii
	// and transform.
	for (auto It = TrackedLidars.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key.Get()))
		{
			It.RemoveCurrent();
			continue;
		}

		if (bAutoAddObjects)
			AGX_SensorEnvironment_helpers::UpdateCollisionSphere(It->Key.Get(), It->Value.Get());
	}
}

void AAGX_SensorEnvironment::UpdateTrackedMeshes()
{
	check(bAutoAddObjects);
	AGX_SensorEnvironment_helpers::UpdateTrackedMeshes(TrackedMeshes);
}

void AAGX_SensorEnvironment::UpdateTrackedAGXMeshes()
{
	check(bAutoAddObjects);
	AGX_SensorEnvironment_helpers::UpdateTrackedMeshes(TrackedAGXMeshes);
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

	auto InstancedMesh = Cast<UInstancedStaticMeshComponent>(OtherComp);
	if (InstancedMesh != nullptr)
	{
		OnLidarBeginOverlapInstancedStaticMeshComponent(*InstancedMesh, OtherBodyIndex);
		return;
	}

	auto Mesh = Cast<UStaticMeshComponent>(OtherComp);
	if (Mesh != nullptr)
	{
		OnLidarBeginOverlapStaticMeshComponent(*Mesh);
		return;
	}

	auto SimpleMesh = Cast<UAGX_SimpleMeshComponent>(OtherComp);
	if (SimpleMesh != nullptr)
	{
		OnLidarBeginOverlapAGXMeshComponent(*SimpleMesh);
		return;
	}
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapStaticMeshComponent(UStaticMeshComponent& Mesh)
{
	FAGX_MeshEntityData* MeshEntityData = TrackedMeshes.Find(&Mesh);
	if (MeshEntityData == nullptr)
		AddMesh(&Mesh);
	else
		MeshEntityData->EntityData.RefCount++;
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapInstancedStaticMeshComponent(
	UInstancedStaticMeshComponent& Mesh, int32 Index)
{
	auto InstancedMeshData = TrackedInstancedMeshes.Find(&Mesh);
	if (InstancedMeshData == nullptr)
	{
		AddInstancedMeshInstance(&Mesh, Index);
		return;
	}

	auto InstancedEntityData = InstancedMeshData->EntitiesData.Find(Index);
	if (InstancedEntityData == nullptr)
		AddInstancedMeshInstance(&Mesh, Index);
	else
		InstancedEntityData->RefCount++;
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_MeshEntityData* MeshEntityData = TrackedAGXMeshes.Find(&Mesh);
	if (MeshEntityData == nullptr)
		AddAGXMesh(&Mesh);
	else
		MeshEntityData->EntityData.RefCount++;
}

void AAGX_SensorEnvironment::OnLidarEndOverlapComponent(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	auto InstancedMesh = Cast<UInstancedStaticMeshComponent>(OtherComp);
	if (InstancedMesh != nullptr)
	{
		OnLidarEndOverlapInstancedStaticMeshComponent(*InstancedMesh, OtherBodyIndex);
		return;
	}

	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(OtherComp);
	if (Mesh != nullptr)
	{
		OnLidarEndOverlapStaticMeshComponent(*Mesh);
		return;
	}

	auto SimpleMesh = Cast<UAGX_SimpleMeshComponent>(OtherComp);
	if (SimpleMesh != nullptr)
	{
		OnLidarEndOverlapAGXMeshComponent(*SimpleMesh);
		return;
	}
}

void AAGX_SensorEnvironment::OnLidarEndOverlapStaticMeshComponent(UStaticMeshComponent& Mesh)
{
	FAGX_MeshEntityData* MeshEntityData = TrackedMeshes.Find(&Mesh);
	if (MeshEntityData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX_SensorEnvironment '%s' failed to track Static Mesh Component '%s'."),
			*GetName(), *Mesh.GetName());
		return;
	}

	AGX_CHECK(MeshEntityData->EntityData.RefCount > 0);
	MeshEntityData->EntityData.RefCount--;
	if (MeshEntityData->EntityData.RefCount == 0)
		TrackedMeshes.Remove(&Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapInstancedStaticMeshComponent(
	UInstancedStaticMeshComponent& Mesh, int32 Index)
{
	auto InstancedMesh = TrackedInstancedMeshes.Find(&Mesh);
	if (InstancedMesh == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"AGX_SensorEnvironment '%s' failed to track Instanced Static Mesh Component '%s'."),
			*GetName(), *Mesh.GetName());
		return;
	}

	auto InstancedEntityData = InstancedMesh->EntitiesData.Find(Index);
	if (InstancedEntityData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX_SensorEnvironment '%s' failed to track Instance %d of Static Mesh Component "
				 "'%s'."),
			*GetName(), Index, *Mesh.GetName());
		return;
	}

	AGX_CHECK(InstancedEntityData->RefCount > 0);
	InstancedEntityData->RefCount--;
	if (InstancedEntityData->RefCount == 0)
		InstancedMesh->EntitiesData.Remove(Index);

	// Finally, we should remove the Instanced Static Mesh Component completely if no instances are
	// tracked.
	if (InstancedMesh->EntitiesData.Num() == 0)
		RemoveInstancedMesh(&Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_MeshEntityData* MeshEntityData = TrackedAGXMeshes.Find(&Mesh);
	if (MeshEntityData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX_SensorEnvironment '%s' failed to track AGX Mesh Component '%s'."),
			*GetName(), *Mesh.GetName());
		return;
	}

	AGX_CHECK(MeshEntityData->EntityData.RefCount > 0);
	MeshEntityData->EntityData.RefCount--;
	if (MeshEntityData->EntityData.RefCount == 0)
		TrackedAGXMeshes.Remove(&Mesh);
}
