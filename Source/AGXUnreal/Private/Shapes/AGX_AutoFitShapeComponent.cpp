#include "Shapes/AGX_AutoFitShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_MeshUtilities.h"

void UAGX_AutoFitShapeComponent::AutoFitToMesh()
{
	TArray<FVector>Vertices;
	TArray<FTriIndices> Indices;
	if (!GetStaticMeshCollisionData(Vertices, Indices))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not auto-fit %s to mesh because no collision data could be extracted."),
			*GetName());
		return;
	}

	AutoFit(Vertices);
}

bool UAGX_AutoFitShapeComponent::GetStaticMeshCollisionData(
	TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices) const
{
	UStaticMesh* StaticMesh = nullptr;
	FTransform StaticMeshWorldTransform;

	if (!AGX_MeshUtilities::FindStaticMeshRelativeToComponent(
			*this, MeshSourceLocation, MeshSourceAsset, StaticMesh, &StaticMeshWorldTransform))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetStaticMeshCollisionData failed in %s. Unable to find static Mesh."),
			*GetName());
		return false;
	}

	check(StaticMesh != nullptr);
	const FTransform ComponentTransform(GetComponentRotation(), GetComponentLocation());
	return AGX_MeshUtilities::GetStaticMeshCollisionData(
		*StaticMesh, StaticMeshWorldTransform, ComponentTransform, OutVertices, OutIndices);
}
