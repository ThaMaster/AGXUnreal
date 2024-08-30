#include "Sensors/AGX_SensorEnvironment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "AGX_SimpleMeshComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_SensorEnvironmentSpriteComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include <algorithm>

namespace AGX_SensorEnvironment_helpers
{
	float GetReflectivityOrDefault(UMaterialInterface* MaterialInterface, float DefaultReflectivity)
	{
		if (MaterialInterface == nullptr)
			return DefaultReflectivity;

		FMaterialParameterInfo Info;
		Info.Name = TEXT("AGXLidarReflectivity");
		float Reflectivity;
		if (!MaterialInterface->GetScalarParameterValue(Info, Reflectivity))
			return DefaultReflectivity;

		return Reflectivity;
	}

	float GetReflectivityOrDefault(UMeshComponent* Mesh, float DefaultReflectivity)
	{
		if (Mesh == nullptr)
			return DefaultReflectivity;

		return GetReflectivityOrDefault(Mesh->GetMaterial(0), DefaultReflectivity);
	}

	float GetReflectivityOrDefault(AAGX_Terrain* Terrain, float DefaultReflectivity)
	{
		if (Terrain == nullptr)
			return DefaultReflectivity;

		const auto TerrainMat = Terrain->TerrainMaterial;
		if (TerrainMat == nullptr)
			return DefaultReflectivity;

		return TerrainMat->LidarReflectivity;
	}

	bool GetVerticesIndices(
		UStaticMeshComponent* Mesh, TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices,
		int32 Lod)
	{
		if (Mesh == nullptr)
			return false;

		const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
		if (StaticMesh == nullptr)
			return false;

		// Default LOD is LodMax, if not set explicitly.
		const uint32 LodMax = StaticMesh->GetNumLODs() - 1;
		const uint32 LodIndex = Lod < 0 ? LodMax : std::min(static_cast<uint32>(Lod), LodMax);
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

		// Chosen arbitrarily, too large will cause Unreal warnings/errors.
		static constexpr double MaxRadius = 1.0e8;
		if (Lidar->Range.Max > MaxRadius)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Lidar %s has a Max Range of %f, but the maximum supported Range is %f. Using "
					 "%f."),
				*Lidar->GetName(), Lidar->Range.Max.GetValue(), MaxRadius, MaxRadius);
		}

		const float Radius = std::min(Lidar->Range.Max.GetValue(), MaxRadius);
		if (!FMath::IsNearlyEqual(Sphere->GetUnscaledSphereRadius(), Radius))
		{
			Sphere->SetSphereRadius(Radius, /*bUpdateOverlaps*/ true);
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
			if (CompTransform.Equals(It->Value.InstanceData.Transform))
				continue;

			It->Value.InstanceData.SetTransform(CompTransform);
		}
	}
}

AAGX_SensorEnvironment::AAGX_SensorEnvironment()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<UAGX_SensorEnvironmentSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
}

bool AAGX_SensorEnvironment::AddMesh(UStaticMeshComponent* Mesh, int32 InLod)
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
	const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
	const bool Res =
		AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds, Lod);
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

bool AAGX_SensorEnvironment::AddInstancedMesh(UInstancedStaticMeshComponent* Mesh, int32 InLod)
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
		const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
		if (AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVertices, OutIndices, Lod))
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
	UInstancedStaticMeshComponent* Mesh, int32 Index, int32 InLod)
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
		const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
		if (AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVertices, OutIndices, Lod))
			AddInstancedMesh(Mesh, OutVertices, OutIndices);
		else
			return false;
	}

	return AddInstancedMeshInstance_Internal(Mesh, Index);
}

bool AAGX_SensorEnvironment::AddMesh(
	UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	using namespace AGX_SensorEnvironment_helpers;
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedMeshes.Contains(Mesh))
		return false;

	FAGX_RtShapeInstanceData& ShapeInstance = TrackedMeshes.Add(Mesh, FAGX_RtShapeInstanceData());
	ShapeInstance.Shape.AllocateNative(Vertices, Indices);
	ShapeInstance.InstanceData.Instance.AllocateNative(
		ShapeInstance.Shape, NativeBarrier, GetReflectivityOrDefault(Mesh, DefaultReflectivity));
	ShapeInstance.InstanceData.SetTransform(Mesh->GetComponentTransform());
	return true;
}

bool AAGX_SensorEnvironment::AddMesh(
	UAGX_SimpleMeshComponent* Mesh, const TArray<FVector>& Vertices,
	const TArray<FTriIndices>& Indices)
{
	using namespace AGX_SensorEnvironment_helpers;
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedAGXMeshes.Contains(Mesh))
		return false;

	FAGX_RtShapeInstanceData& ShapeInstance =
		TrackedAGXMeshes.Add(Mesh, FAGX_RtShapeInstanceData());
	ShapeInstance.Shape.AllocateNative(Vertices, Indices);
	ShapeInstance.InstanceData.Instance.AllocateNative(
		ShapeInstance.Shape, NativeBarrier, GetReflectivityOrDefault(Mesh, DefaultReflectivity));
	ShapeInstance.InstanceData.SetTransform(Mesh->GetComponentTransform());
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

	auto& InstancedShapeInstance =
		TrackedInstancedMeshes.Add(Mesh, FAGX_RtInstancedShapeInstanceData());
	InstancedShapeInstance.Shape.AllocateNative(Vertices, Indices);
	AGX_CHECK(InstancedShapeInstance.Shape.HasNative());
	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMeshInstance_Internal(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	using namespace AGX_SensorEnvironment_helpers;
	AGX_CHECK(HasNative());
	AGX_CHECK(Mesh != nullptr);
	AGX_CHECK(Mesh->IsValidInstance(Index));

	FAGX_RtInstancedShapeInstanceData* InstancedShapeInstance = TrackedInstancedMeshes.Find(Mesh);

	// This function should only be called for known Instanced Static Mesh Components.
	AGX_CHECK(InstancedShapeInstance != nullptr);
	if (InstancedShapeInstance == nullptr)
		return false;

	if (InstancedShapeInstance->InstancesData.Contains(Index))
		return false; // We already track this instance.

	FAGX_RtInstanceData& InstanceData =
		InstancedShapeInstance->InstancesData.Add(Index, FAGX_RtInstanceData());
	InstanceData.Instance.AllocateNative(
		InstancedShapeInstance->Shape, NativeBarrier,
		GetReflectivityOrDefault(Mesh, DefaultReflectivity));
	AGX_CHECK(InstanceData.Instance.HasNative());
	FTransform InstanceTrans;
	Mesh->GetInstanceTransform(Index, InstanceTrans, true);
	InstanceData.SetTransform(InstanceTrans);
	return true;
}

bool AAGX_SensorEnvironment::AddTerrain(AAGX_Terrain* Terrain)
{
	using namespace AGX_SensorEnvironment_helpers;
	if (!HasNative() || Terrain == nullptr)
		return false;

	if (Terrain->bEnableTerrainPaging)
	{
		FTerrainPagerBarrier* PagerBarrier = Terrain->GetOrCreateNativeTerrainPager();
		if (PagerBarrier == nullptr)
			return false;

		return NativeBarrier.Add(
			*PagerBarrier, GetReflectivityOrDefault(Terrain, DefaultReflectivity));
	}
	else
	{
		FTerrainBarrier* TerrainBarrier = Terrain->GetOrCreateNative();
		if (TerrainBarrier == nullptr)
			return false;

		return NativeBarrier.Add(
			*TerrainBarrier, GetReflectivityOrDefault(Terrain, DefaultReflectivity));
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

	if (!InstancedMeshData->InstancesData.Contains(Index))
		return false;

	InstancedMeshData->InstancesData.Remove(Index);
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

void AAGX_SensorEnvironment::Tick(float DeltaSeconds)
{
	UpdateTrackedLidars();
	UpdateTrackedMeshes();

	if (UpdateAddedInstancedMeshesTransforms)
		UpdateTrackedInstancedMeshes();

	UpdateTrackedAGXMeshes();
	StepTrackedLidars();
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

				// = true yields bugs of mutliple begin/end overlaps. See internal issue 957.
				CollSph->bTraceComplexOnMove = false;

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
	AGX_SensorEnvironment_helpers::UpdateTrackedMeshes(TrackedMeshes);
}

void AAGX_SensorEnvironment::UpdateTrackedInstancedMeshes()
{
	for (auto It = TrackedInstancedMeshes.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key.Get()))
		{
			It.RemoveCurrent();
			continue;
		}

		// Instance.
		for (auto Ite = It->Value.InstancesData.CreateIterator(); Ite; ++Ite)
		{
			if (!It->Key->IsValidInstance(Ite->Key))
			{
				Ite.RemoveCurrent();
				continue;
			}

			FTransform InstanceTransform;
			It->Key->GetInstanceTransform(Ite->Key, InstanceTransform, true);
			if (InstanceTransform.Equals(Ite->Value.Transform))
				continue;

			Ite->Value.SetTransform(InstanceTransform);
		}
	}
}

void AAGX_SensorEnvironment::UpdateTrackedAGXMeshes()
{
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

	if (auto SceneComponent = Cast<USceneComponent>(OtherComp))
	{
		if (bIgnoreInvisibleObjects && !SceneComponent->ShouldRender())
			return;
	}

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
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		AddMesh(&Mesh, DefaultLODIndex);
	else
		ShapeInstanceData->InstanceData.RefCount++;
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapInstancedStaticMeshComponent(
	UInstancedStaticMeshComponent& Mesh, int32 Index)
{
	auto InstancedMeshData = TrackedInstancedMeshes.Find(&Mesh);
	if (InstancedMeshData == nullptr)
	{
		AddInstancedMeshInstance(&Mesh, Index, DefaultLODIndex);
		return;
	}

	auto InstanceData = InstancedMeshData->InstancesData.Find(Index);
	if (InstanceData == nullptr)
		AddInstancedMeshInstance(&Mesh, Index, DefaultLODIndex);
	else
		InstanceData->RefCount++;
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedAGXMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		AddAGXMesh(&Mesh);
	else
		ShapeInstanceData->InstanceData.RefCount++;
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
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		return;

	AGX_CHECK(ShapeInstanceData->InstanceData.RefCount > 0);
	ShapeInstanceData->InstanceData.RefCount--;
	if (ShapeInstanceData->InstanceData.RefCount == 0)
		TrackedMeshes.Remove(&Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapInstancedStaticMeshComponent(
	UInstancedStaticMeshComponent& Mesh, int32 Index)
{
	if (!Mesh.IsValidInstance(Index))
		return;

	auto InstancedShape = TrackedInstancedMeshes.Find(&Mesh);
	if (InstancedShape == nullptr)
		return;

	auto InstanceData = InstancedShape->InstancesData.Find(Index);
	if (InstanceData == nullptr)
		return;

	AGX_CHECK(InstanceData->RefCount > 0);
	InstanceData->RefCount--;
	if (InstanceData->RefCount == 0)
		InstancedShape->InstancesData.Remove(Index);

	// Finally, we should remove the Instanced Static Mesh Component completely if no instances are
	// tracked.
	if (InstancedShape->InstancesData.Num() == 0)
		RemoveInstancedMesh(&Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedAGXMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		return;

	AGX_CHECK(ShapeInstanceData->InstanceData.RefCount > 0);
	ShapeInstanceData->InstanceData.RefCount--;
	if (ShapeInstanceData->InstanceData.RefCount == 0)
		TrackedAGXMeshes.Remove(&Mesh);
}
