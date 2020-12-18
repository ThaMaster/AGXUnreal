#include "Shapes/AGX_TrimeshShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Rendering/PositionVertexBuffer.h"
#include "RenderingThread.h"

UAGX_TrimeshShapeComponent::UAGX_TrimeshShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MeshSourceLocation = TSL_PARENT_STATIC_MESH_COMPONENT;
	bOverrideMeshSourceLodIndex = true;
	MeshSourceLodIndex = 0;
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

void UAGX_TrimeshShapeComponent::CopyFrom(const FTrimeshShapeBarrier& Barrier)
{
	/// \todo Where is all the triangle data copied?
	Super::CopyFrom(Barrier);
}

void UAGX_TrimeshShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& /*OutMeshData*/)
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
		   MemberPropertyName ==
			   GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation) ||
		   MemberPropertyName ==
			   GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset);
}

bool UAGX_TrimeshShapeComponent::CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	const UProperty* InProperty
#else
	const FProperty* InProperty
#endif
) const
{
	if (InProperty->GetFName() ==
		GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset))
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
	check(!HasNative());

	TArray<FVector> Vertices;
	TArray<FTriIndices> Indices;
	if (GetStaticMeshCollisionData(Vertices, Indices))
	{
		NativeBarrier.AllocateNative(Vertices, Indices, /*bClockwise*/ false, GetName());
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("TrimeshShapeComponent '%s' does not have a StaticMeshComponent to read triangle "
				 "data from. The generated native shape will be invalid."),
			*GetName());
		NativeBarrier.AllocateNative({}, {}, /*bClockwise*/ false, GetName());
	}

	UpdateNativeProperties();
}

void UAGX_TrimeshShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

bool UAGX_TrimeshShapeComponent::FindStaticMeshSource(
	UStaticMesh*& StaticMesh, FTransform* WorldTransform) const
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

static int32 AddCollisionVertex(
	const FVector& VertexPosition, const FTransform& Transform, TArray<FVector>& CollisionVertices,
	TMap<FVector, int32>& MeshToCollisionVertexIndices)
{
	if (int32* CollisionVertexIndexPtr = MeshToCollisionVertexIndices.Find(VertexPosition))
	{
		// Already been added once, so just return the index.
		return *CollisionVertexIndexPtr;
	}
	else
	{
		// Copy position from mesh to collision data.
		int CollisionVertexIndex =
			CollisionVertices.Add(Transform.TransformPosition(VertexPosition));

		// Add collision index to map.
		MeshToCollisionVertexIndices.Add(VertexPosition, CollisionVertexIndex);

		return CollisionVertexIndex;
	}
}

namespace
{
	bool CopyMeshBuffers(
		const FStaticMeshLODResources& Mesh, TArray<uint32>& OutIndices,
		TArray<FVector>& OutVertices)
	{
		bool Result = false;
		const uint32 NumIndices = static_cast<uint32>(Mesh.IndexBuffer.GetNumIndices());
		const uint32 NumVertices = Mesh.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		OutIndices.Reserve(static_cast<size_t>(NumIndices));
		OutVertices.Reserve(static_cast<size_t>(NumVertices));

		// In cooked builds the Mesh data can only be accessed from the rendering thread.
		ENQUEUE_RENDER_COMMAND(FCopyMeshBuffers)
		([&](FRHICommandListImmediate& RHICmdList) {
			auto& IndexBufferRHI = Mesh.IndexBuffer.IndexBufferRHI;
			auto& VertexBufferRHI = Mesh.VertexBuffers.PositionVertexBuffer.VertexBufferRHI;

			// Check buffer sizes.
			uint32 NumIndicesRHI = IndexBufferRHI->GetSize() / sizeof(uint16);
			uint32 NumVerticesRHI = VertexBufferRHI->GetSize() / sizeof(FVector);
			if (NumIndices != NumIndicesRHI || NumVertices != NumVerticesRHI)
			{
				Result = false;
				return;
			}

			// Copy index buffer.
			if (IndexBufferRHI->GetStride() == 2)
			{
				// Two byte index size.
				uint16* IndexBufferData = static_cast<uint16*>(
					RHILockIndexBuffer(IndexBufferRHI, 0, IndexBufferRHI->GetSize(), RLM_ReadOnly));
				for (uint32 i = 0; i < NumIndices; i++)
				{
					OutIndices.Add(static_cast<uint32>(IndexBufferData[i]));
				}
			}
			else
			{
				// Four byte index size (stride must be either 2 or 4).
				uint32* IndexBufferData = static_cast<uint32*>(
					RHILockIndexBuffer(IndexBufferRHI, 0, IndexBufferRHI->GetSize(), RLM_ReadOnly));
				for (uint32 i = 0; i < NumIndices; i++)
				{
					OutIndices.Add(IndexBufferData[i]);
				}
			}
			RHIUnlockIndexBuffer(Mesh.IndexBuffer.IndexBufferRHI);

			// Copy vertex buffer.
			FVector* VertexBufferData = static_cast<FVector*>(
				RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferRHI->GetSize(), RLM_ReadOnly));
			for (uint32 i = 0; i < NumVertices; i++)
			{
				OutVertices.Add(VertexBufferData[i]);
			}
			RHIUnlockVertexBuffer(Mesh.VertexBuffers.PositionVertexBuffer.VertexBufferRHI);
			Result = true;
		});

		// Wait for rendering thread to finish.
		FlushRenderingCommands();
		return Result;
	}
}

bool UAGX_TrimeshShapeComponent::GetStaticMeshCollisionData(
	TArray<FVector>& Vertices, TArray<FTriIndices>& Indices) const
{
	// NOTE: Code below is very similar to UStaticMesh::GetPhysicsTriMeshData,
	// only with some simplifications, so one can check that implementation for reference.
	// One important difference is that we hash on vertex position instead of index because we
	// want to re-merge vertices that has been split in the rendering data.

	UStaticMesh* StaticMesh = nullptr;
	FTransform MeshWorldTransform;

	if (!FindStaticMeshSource(StaticMesh, &MeshWorldTransform))
		return false;

	const FTransform ComponentTransformNoScale =
		FTransform(GetComponentRotation(), GetComponentLocation());

	// Final vertex positions need to be relative to this Agx Geometry,
	// and any scale needs to be baked into the positions, because AGX
	// does not support scale.
	const FTransform RelativeTransform =
		MeshWorldTransform.GetRelativeTransform(ComponentTransformNoScale);

	const uint32 LodIndex = FMath::Clamp<int32>(
		bOverrideMeshSourceLodIndex ? MeshSourceLodIndex : StaticMesh->LODForCollision, 0,
		StaticMesh->GetNumLODs() - 1);

	if (!StaticMesh->HasValidRenderData(/*bCheckLODForVerts*/ true, LodIndex))
		return false;

	const FStaticMeshLODResources& Mesh = StaticMesh->GetLODForExport(/*LODIndex*/ LodIndex);
	TMap<FVector, int32> MeshToCollisionVertexIndices;

	// Copy the Index and Vertex buffers from the mesh.
	TArray<uint32> IndexBuffer;
	TArray<FVector> VertexBuffer;
	const bool CopyResult = CopyMeshBuffers(Mesh, IndexBuffer, VertexBuffer);
	if (CopyResult == false || IndexBuffer.Num() == 0 || VertexBuffer.Num() == 0)
	{
		return false;
	}

	const uint32 NumIndices = static_cast<uint32>(IndexBuffer.Num());
	for (int32 SectionIndex = 0; SectionIndex < Mesh.Sections.Num(); ++SectionIndex)
	{
		const FStaticMeshSection& Section = Mesh.Sections[SectionIndex];
		const uint32 OnePastLastIndex = Section.FirstIndex + Section.NumTriangles * 3;

		for (uint32 Index = Section.FirstIndex; Index < OnePastLastIndex; Index += 3)
		{
			if (Index + 2 >= NumIndices)
			{
				break;
			}

			const uint32 IndexFirst = IndexBuffer[Index];
			const uint32 IndexSecond = IndexBuffer[Index + 1];
			const uint32 IndexThird = IndexBuffer[Index + 2];
			FTriIndices Triangle;

			Triangle.v0 = AddCollisionVertex(
				VertexBuffer[IndexFirst], RelativeTransform, Vertices,
				MeshToCollisionVertexIndices);
			Triangle.v1 = AddCollisionVertex(
				VertexBuffer[IndexSecond], RelativeTransform, Vertices,
				MeshToCollisionVertexIndices);
			Triangle.v2 = AddCollisionVertex(
				VertexBuffer[IndexThird], RelativeTransform, Vertices,
				MeshToCollisionVertexIndices);

			Indices.Add(Triangle);
		}
	}

	return Vertices.Num() > 0 && Indices.Num() > 0;
}
