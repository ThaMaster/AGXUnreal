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

void UAGX_TrimeshShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bIsSensor)))
	{
		if (UMeshComponent* Mesh = FindMeshComponent(MeshSourceLocation))
		{
			bIsSensor ? ApplySensorMaterial(*Mesh) : RemoveSensorMaterial(*Mesh);
		}
	}
	else if (PropertyChangedEvent.GetPropertyName().IsEqual(
				 GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation)))
	{
		if (UMeshComponent* Mesh = FindMeshComponent(MeshSourceLocation))
		{
			bIsSensor ? ApplySensorMaterial(*Mesh) : RemoveSensorMaterial(*Mesh);
		}
	}
}

void UAGX_TrimeshShapeComponent::PreEditChange(FProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);

	if (PropertyThatWillChange->GetName().Equals(
			GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation).ToString()))
	{
		if (!bIsSensor)
		{
			return;
		}

		// Clean up sensor material of the old source MeshComponent if applied.
		if (UMeshComponent* Mesh = FindMeshComponent(MeshSourceLocation))
		{
			RemoveSensorMaterial(*Mesh);
		}
	}
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

UMeshComponent* UAGX_TrimeshShapeComponent::FindMeshComponent(
	TEnumAsByte<EAGX_TrimeshSourceLocation> InMeshSourceLocation) const
{
	if (InMeshSourceLocation == TSL_CHILD_STATIC_MESH_COMPONENT)
	{
		TArray<USceneComponent*> Children;
		GetChildrenComponents(/*bIncludeAllDescendants*/ false, Children);

		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			if (USceneComponent* Child = Children[ChildIndex])
			{
				if (Child->IsA(UStaticMeshComponent::StaticClass()))
				{
					return Cast<UStaticMeshComponent>(Child);
				}
			}
		}
	}
	else if (InMeshSourceLocation == TSL_PARENT_STATIC_MESH_COMPONENT)
	{
		TArray<USceneComponent*> Ancestors;
		GetParentComponents(Ancestors);

		for (int32 AncestorIndex = 0; AncestorIndex < Ancestors.Num(); ++AncestorIndex)
		{
			if (USceneComponent* Ancestor = Ancestors[AncestorIndex])
			{
				if (Ancestor->IsA(UStaticMeshComponent::StaticClass()))
				{
					return Cast<UStaticMeshComponent>(Ancestor);
				}
			}
		}
	}

	return nullptr;
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
	/**
	 * Read triangle mesh data directly from the Static Mesh asset.
	 *
	 * This only works in Editor builds, and for meshes that has the Allow CPU Access flag set. In
	 * particular, it does not work for cooked builds.
	 *
	 * If neither of the above is true then the mesh data must be read using
	 * CopyMeshBuffesRenderThread.
	 *
	 * @param Mesh The mesh to read triangles from.
	 * @param OutPositions Array to which the vertex locations are written.
	 * @param OutIndices Array to which the vertex indices are written.
	 */
	void CopyMeshBuffersGameThread(
		const FStaticMeshLODResources& Mesh, TArray<FVector>& OutPositions,
		TArray<uint32>& OutIndices)
	{
		// Copy positions.
		const FPositionVertexBuffer& MeshPositions = Mesh.VertexBuffers.PositionVertexBuffer;
		const uint32 NumPositions = static_cast<uint32>(MeshPositions.GetNumVertices());
		OutPositions.Reserve(NumPositions);
		for (uint32 I = 0; I < NumPositions; ++I)
		{
			OutPositions.Add(MeshPositions.VertexPosition(I));
		}

		// Copy indices.
		const FIndexArrayView MeshIndices = Mesh.IndexBuffer.GetArrayView();
		const int32 NumIndices = MeshIndices.Num();
		check(MeshIndices.Num() == Mesh.IndexBuffer.GetNumIndices()); /// \todo Remove this line.
		OutIndices.Reserve(NumIndices);
		for (int32 I = 0; I < NumIndices; ++I)
		{
			check(MeshIndices[I] < NumPositions);
			OutIndices.Add(MeshIndices[I]);
		}
	}

	/**
	 * Enqueue, and wait for the completion of, a render command to copy the triangle mesh data from
	 * graphics memory to host memory.
	 *
	 * This approach is required for cooked builds unless the Static Mesh asset has the Allow CPU
	 * Access flag set.
	 *
	 * @todo This doesn't work on Linux. We get indices that point outside of the vertex buffer,
	 * which should never happen.
	 *
	 * @param Mesh The mesh to read triangle mesh data from.
	 * @param OutPositions Array to which the vertex positions are written.
	 * @param OutIndices Array to which the vertex indices are written.
	 */
	void CopyMeshBuffersRenderThread(
		const FStaticMeshLODResources& Mesh, TArray<FVector>& OutPositions,
		TArray<uint32>& OutIndices)
	{
		const uint32 NumIndices = static_cast<uint32>(Mesh.IndexBuffer.GetNumIndices());
		OutIndices.Reserve(NumIndices);

		const uint32 NumPositions = Mesh.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		OutPositions.Reserve(NumPositions);

		ENQUEUE_RENDER_COMMAND(FCopyMeshBuffers)
		([&](FRHICommandListImmediate& RHICmdList) {
			// Copy vertex buffer.
			auto& PositionRHI = Mesh.VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
			const uint32 NumPositionBytes = PositionRHI->GetSize();
			FVector* PositionData = static_cast<FVector*>(
				RHILockVertexBuffer(PositionRHI, 0, NumPositionBytes, RLM_ReadOnly));
			for (uint32 I = 0; I < NumPositions; I++)
			{
				OutPositions.Add(PositionData[I]);
			}
			RHIUnlockVertexBuffer(PositionRHI);

			// Copy index buffer.
			auto& IndexRHI = Mesh.IndexBuffer.IndexBufferRHI;
			if (IndexRHI->GetStride() == 2)
			{
				// Two byte index size.
				uint16* IndexData = static_cast<uint16*>(
					RHILockIndexBuffer(IndexRHI, 0, IndexRHI->GetSize(), RLM_ReadOnly));
				for (uint32 i = 0; i < NumIndices; i++)
				{
					check(IndexData[i] < NumPositions);
					OutIndices.Add(static_cast<uint32>(IndexData[i]));
				}
			}
			else
			{
				// Four byte index size (stride must be either 2 or 4).
				check(IndexRHI->GetStride() == 4);
				uint32* IndexData = static_cast<uint32*>(
					RHILockIndexBuffer(IndexRHI, 0, IndexRHI->GetSize(), RLM_ReadOnly));
				for (uint32 i = 0; i < NumIndices; i++)
				{
					check(IndexData[i] < NumPositions);
					OutIndices.Add(IndexData[i]);
				}
			}
			RHIUnlockIndexBuffer(IndexRHI);
		});

		// Wait for rendering thread to finish.
		FlushRenderingCommands();
	}

	void CopyMeshBuffers(
		const FStaticMeshLODResources& Mesh, TArray<FVector>& OutPositions,
		TArray<uint32>& OutIndices)
	{
#if WITH_EDITOR
		CopyMeshBuffersGameThread(Mesh, OutPositions, OutIndices);
#else
		CopyMeshBuffersRenderThread(Mesh, OutPositions, OutIndices);
#endif
	}

	/**
	 * Check that CopyMeshBuffersGameThread and CopyMeshBuffersRenderThread produces the same
	 * result. On Linux it doesn't, which is bad.
	 *
	 * @param Mesh The mesh to read trimesh data from.
	 */
	void CheckMeshBufferValidity(const FStaticMeshLODResources& Mesh)
	{
		// We trust the game thread version, so just call it.
		TArray<FVector> GamePositions;
		TArray<uint32> GameIndices;
		CopyMeshBuffersGameThread(Mesh, GamePositions, GameIndices);

		const int32 TrueNumIndices = GameIndices.Num();
		const int32 TrueNumPositions = GamePositions.Num();

		// We don't trust the render thread version, so copy-pasted into here with some additional
		// test/verification code.

		auto& PositionBuffer = Mesh.VertexBuffers.PositionVertexBuffer;

		const uint32 NumPositions = PositionBuffer.GetNumVertices();
		if (NumPositions != TrueNumPositions)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Got wrong number of locations from "
					 "Mesh.VertexBuffers.PositionVertexBuffer.GetNumVertices()."));
		}

		auto& PositionRhi = PositionBuffer.VertexBufferRHI;
		uint32 PositionRhiSize = PositionRhi->GetSize();
		uint32 PositionRhiCount = PositionRhiSize / sizeof(FVector);
		if (PositionRhiCount != TrueNumPositions)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Got wrong number of locations from PositionRhi->GetSize() / sizeof(FVector)."))
		}

		TArray<FVector> RenderPositions;
		RenderPositions.Reserve(NumPositions);

		auto& IndexBuffer = Mesh.IndexBuffer;
		const int32 NumIndices = IndexBuffer.GetNumIndices();
		if (NumIndices != TrueNumIndices)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Got wrong number of indices from Mesh.IndexBuffer.GetNumIndices()."));
		}

		auto& IndexRhi = Mesh.IndexBuffer.IndexBufferRHI;
		uint32 IndexRhiSize = IndexRhi->GetSize();
		uint32 IndexRhiCount = IndexRhiSize / IndexRhi->GetStride();
		if (IndexRhiCount != TrueNumIndices)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Got wrong number of indices from IndexRhi->GetSize() / sizeof(uint16)."));
		}

		TArray<uint32> RenderIndices;
		RenderIndices.Reserve(NumIndices);

		ENQUEUE_RENDER_COMMAND(FCopyMeshBuffers)
		([&](FRHICommandListImmediate& RHICmdList) {
			// Copy position buffer.
			FVector* PositionData = static_cast<FVector*>(
				RHILockVertexBuffer(PositionRhi, 0, PositionRhi->GetSize(), RLM_ReadOnly));
			for (uint32 i = 0; i < NumPositions; i++)
			{
				RenderPositions.Add(PositionData[i]);
			}
			RHIUnlockVertexBuffer(PositionRhi);

			// Copy index buffer.
			if (IndexRhi->GetStride() == 2)
			{
				// Two byte index size.
				uint16* IndexBufferData = static_cast<uint16*>(
					RHILockIndexBuffer(IndexRhi, 0, IndexRhi->GetSize(), RLM_ReadOnly));
				for (int32 i = 0; i < NumIndices; i++)
				{
					RenderIndices.Add(static_cast<uint32>(IndexBufferData[i]));
				}
			}
			else
			{
				// Four byte index size (stride must be either 2 or 4).
				check(IndexRhi->GetStride() == 4);
				uint32* IndexData = static_cast<uint32*>(
					RHILockIndexBuffer(IndexRhi, 0, IndexRhi->GetSize(), RLM_ReadOnly));
				for (int32 i = 0; i < NumIndices; i++)
				{
					RenderIndices.Add(IndexData[i]);
				}
			}
			RHIUnlockIndexBuffer(IndexRhi);
		});

		// Wait for rendering thread to finish.
		FlushRenderingCommands();

		// Check if the render thread copy got the same data as the game thread copy.
		int32 NumIndexMismatch {0};
		for (int I = 0; I < FMath::Min(TrueNumIndices, NumIndices); ++I)
		{
			uint32 TrueIndex = GameIndices[I];
			uint32 Index = RenderIndices[I];
			if (TrueIndex != Index)
			{
				++NumIndexMismatch;
			}
		}
		if (NumIndexMismatch > 0)
		{
			UE_LOG(LogAGX, Error, TEXT("Got %d mismatched indices."), NumIndexMismatch);
		}

		if (GameIndices != RenderIndices)
		{
			UE_LOG(LogAGX, Error, TEXT("Error reading vertex indices from GPU memory."));
		}
		if (GamePositions != RenderPositions)
		{
			UE_LOG(LogAGX, Error, TEXT("Error reading vertex positions from GPU memory."));
		}
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

	const FStaticMeshLODResources& Mesh = StaticMesh->GetLODForExport(LodIndex);
	TMap<FVector, int32> MeshToCollisionVertexIndices;

	// Copy the Index and Vertex buffers from the mesh.
	TArray<uint32> IndexBuffer;
	TArray<FVector> VertexBuffer;
	CopyMeshBuffers(Mesh, VertexBuffer, IndexBuffer);
	if (IndexBuffer.Num() == 0 || VertexBuffer.Num() == 0)
	{
		return false;
	}

	check(Mesh.IndexBuffer.GetNumIndices() == IndexBuffer.Num());
	check(Mesh.VertexBuffers.PositionVertexBuffer.GetNumVertices() == VertexBuffer.Num());

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
