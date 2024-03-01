#include "Sensors/AGX_SensorEnvironment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "Sensors/AGX_SensorEnvironmentSpriteComponent.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
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

bool AAGX_SensorEnvironment::Add(UStaticMeshComponent* Mesh)
{
	return AGX_SensorEnvironment_helpers::Add(*this, Mesh);
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
			GET_MEMBER_NAME_CHECKED(ThisClass, Lidars)};

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
}

void AAGX_SensorEnvironment::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
