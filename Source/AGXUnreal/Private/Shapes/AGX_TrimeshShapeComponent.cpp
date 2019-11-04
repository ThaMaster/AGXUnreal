#include "Shapes/AGX_TrimeshShapeComponent.h"

#include "AGX_LogCategory.h"
#include "Utilities/AGX_MeshUtilities.h"

UAGX_TrimeshShapeComponent::UAGX_TrimeshShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MeshSourceLocation = TSL_PARENT_STATIC_MESH_COMPONENT;
	bOverrideMeshSourceLodIndex = true;
	MeshSourceLodIndex = 0;

	UE_LOG(LogAGX, Log, TEXT("TrimeshShape instance created."));
}

FShapeBarrier* UAGX_TrimeshShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_TrimeshShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_TrimeshShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FTrimeshShapeBarrier* UAGX_TrimeshShapeComponent::GetNativeTrimesh()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_TrimeshShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);
}

void UAGX_TrimeshShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	// Visualized by the Static Mesh.. But maybe we should visualize the
	// internal triangle data anyway, for debug purposes? Especially since
	// we can use lower LOD levels of the Static Mesh for collisions.
}

#if WITH_EDITOR

bool UAGX_TrimeshShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
		   MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation) ||
		   MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset);
}

bool UAGX_TrimeshShapeComponent::CanEditChange(const UProperty* InProperty) const
{
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset))
	{
		return MeshSourceLocation == TSL_STATIC_MESH_ASSET;
	}
	else
	{
		return Super::CanEditChange(InProperty);
	}
}

#endif

void UAGX_TrimeshShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for TrimeshShapeComponent."));
	check(!HasNative());

	TArray<FVector> Vertices;
	TArray<FTriIndices> Indices;
	if (GetStaticMeshCollisionData(Vertices, Indices))
	{
		NativeBarrier.AllocateNative(Vertices, Indices, /*bClockwise*/ false, GetWorld());
	}
	else
	{
		NativeBarrier.AllocateNative({}, {}, /*bClockwise*/ false, GetWorld());
	}

	UpdateNativeProperties();
}

void UAGX_TrimeshShapeComponent::ReleaseNative()
{
	UE_LOG(LogAGX, Log, TEXT("Releasing native object for TrimeshShapeComponent."));
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

bool UAGX_TrimeshShapeComponent::FindStaticMeshSource(UStaticMesh*& StaticMesh, FTransform* WorldTransform) const
{
	if (MeshSourceLocation == TSL_CHILD_STATIC_MESH_COMPONENT)
	{
		TArray<USceneComponent*> Children;
		GetChildrenComponents(/*bIncludeAllDescendants*/ false, Children);

		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			if (USceneComponent* Child = Children[ChildIndex])
			{
				if (Child->IsA(UStaticMeshComponent::StaticClass()))
				{
					StaticMesh = Cast<UStaticMeshComponent>(Child)->GetStaticMesh();
					if (WorldTransform)
						*WorldTransform = Child->GetComponentTransform();
					return true;
				}
			}
		}
	}
	else if (MeshSourceLocation == TSL_PARENT_STATIC_MESH_COMPONENT)
	{
		TArray<USceneComponent*> Ancestors;
		GetParentComponents(Ancestors);

		for (int32 AncestorIndex = 0; AncestorIndex < Ancestors.Num(); ++AncestorIndex)
		{
			if (USceneComponent* Ancestor = Ancestors[AncestorIndex])
			{
				if (Ancestor->IsA(UStaticMeshComponent::StaticClass()))
				{
					StaticMesh = Cast<UStaticMeshComponent>(Ancestor)->GetStaticMesh();
					if (WorldTransform)
						*WorldTransform = Ancestor->GetComponentTransform();
					return true;
				}
			}
		}
	}
	else if (MeshSourceLocation == TSL_STATIC_MESH_ASSET)
	{
		StaticMesh = MeshSourceAsset;
		if (WorldTransform)
			*WorldTransform = GetComponentTransform();
		return true;
	}

	StaticMesh = nullptr;
	if (WorldTransform)
		*WorldTransform = FTransform::Identity;
	return false;
}

static int32 AddCollisionVertex(const int32 MeshVertexIndex, const FPositionVertexBuffer& MeshVertices,
	const FTransform& Transform, TArray<FVector>& CollisionVertices, TMap<int32, int32>& MeshToCollisionVertexIndices)
{
	if (int32* CollisionVertexIndexPtr = MeshToCollisionVertexIndices.Find(MeshVertexIndex))
	{
		// Already been added once, so just return its index.
		return *CollisionVertexIndexPtr;
	}
	else
	{
		// Copy position
		int32 CollisionVertexIndex =
			CollisionVertices.Add(Transform.TransformPosition(MeshVertices.VertexPosition(MeshVertexIndex)));

		// Add index to map.
		MeshToCollisionVertexIndices.Add(MeshVertexIndex, CollisionVertexIndex);

		return CollisionVertexIndex;
	}
}

bool UAGX_TrimeshShapeComponent::GetStaticMeshCollisionData(
	TArray<FVector>& Vertices, TArray<FTriIndices>& Indices) const
{
	// NOTE: Code below is very similar to UStaticMesh::GetPhysicsTriMeshData,
	// only with some simplifications, so one can check that implementation for reference.

	UStaticMesh* StaticMesh = nullptr;
	FTransform MeshWorldTransform;

	if (!FindStaticMeshSource(StaticMesh, &MeshWorldTransform))
		return false;

	const FTransform ComponentTransformNoScale = FTransform(GetComponentRotation(), GetComponentLocation());

	// Final vertex positions need to be relative to this Agx Geometry,
	// and any scale needs to be baked into the positions, because AGX
	// does not support scale.
	const FTransform RelativeTransform = MeshWorldTransform.GetRelativeTransform(ComponentTransformNoScale);

	const uint32 LodIndex =
		FMath::Clamp<int32>(bOverrideMeshSourceLodIndex ? MeshSourceLodIndex : StaticMesh->LODForCollision, 0,
			StaticMesh->GetNumLODs() - 1);

	if (!StaticMesh->HasValidRenderData(/*bCheckLODForVerts*/ true, LodIndex))
		return false;

	const FStaticMeshLODResources& Mesh = StaticMesh->GetLODForExport(/*LODIndex*/ LodIndex);
	FIndexArrayView MeshIndices = Mesh.IndexBuffer.GetArrayView();
	TMap<int32, int32> MeshToCollisionVertexIndices;

	for (int32 SectionIndex = 0; SectionIndex < Mesh.Sections.Num(); ++SectionIndex)
	{
		const FStaticMeshSection& Section = Mesh.Sections[SectionIndex];
		const uint32 OnePastLastIndex = Section.FirstIndex + Section.NumTriangles * 3;

		for (uint32 Index = Section.FirstIndex; Index < OnePastLastIndex; Index += 3)
		{
			FTriIndices Triangle;
			Triangle.v0 = AddCollisionVertex(MeshIndices[Index + 0], Mesh.VertexBuffers.PositionVertexBuffer,
				RelativeTransform, Vertices, MeshToCollisionVertexIndices);
			Triangle.v1 = AddCollisionVertex(MeshIndices[Index + 1], Mesh.VertexBuffers.PositionVertexBuffer,
				RelativeTransform, Vertices, MeshToCollisionVertexIndices);
			Triangle.v2 = AddCollisionVertex(MeshIndices[Index + 2], Mesh.VertexBuffers.PositionVertexBuffer,
				RelativeTransform, Vertices, MeshToCollisionVertexIndices);

			Indices.Add(Triangle);
		}
	}

	return Vertices.Num() > 0 && Indices.Num() > 0;
}
