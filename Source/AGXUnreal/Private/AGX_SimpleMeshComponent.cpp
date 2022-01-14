// Copyright 2022, Algoryx Simulation AB.


#include "AGX_SimpleMeshComponent.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "Engine/CollisionProfile.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "Misc/EngineVersionComparison.h"

/** Scene proxy */
class FAGX_SimpleMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FAGX_SimpleMeshSceneProxy(UAGX_SimpleMeshComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, VertexFactory(GetScene().GetFeatureLevel(), "FAGX_SimpleMeshSceneProxy")
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		check(Component->MeshData);

		const FAGX_SimpleMeshData& MeshData = *Component->MeshData.Get();

		const bool HasTexCoords = MeshData.TexCoords.Num() > 0;
		const bool HasTangents = MeshData.Tangents.Num() > 0;
		const bool HasIndexBuffer = MeshData.Indices.Num() > 0;

		check(MeshData.Vertices.Num() >= 3);
		check(MeshData.Normals.Num() == MeshData.Vertices.Num());
		check(!HasTexCoords || (MeshData.TexCoords.Num() == MeshData.Vertices.Num()));
		check(HasIndexBuffer || (MeshData.Vertices.Num() % 3 == 0));
		check(!HasIndexBuffer || (MeshData.Indices.Num() % 3 == 0));
		check(!HasIndexBuffer || (MeshData.Indices.Num() >= 3));

		const FColor VertexColor(255, 255, 255);
		const int32 NumVertices(MeshData.Vertices.Num());
		const int32 NumIndices((HasIndexBuffer ? MeshData.Indices.Num() : MeshData.Vertices.Num()));
		const int32 NumTriangles = NumIndices / 3;
		uint32 NumTexCoords = 1;
		uint32 LightMapIndex = 0;

		check(NumTexCoords < MAX_STATIC_TEXCOORDS && NumTexCoords > 0);
		check(LightMapIndex < NumTexCoords);

		VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		VertexBuffers.StaticMeshVertexBuffer.Init(NumVertices, NumTexCoords);
		VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Populate Vertex Buffer Resources
		for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
		{
			const FVector VertexPosition = MeshData.Vertices[VertexIndex];
			const FVector2D VertexTexCoord =
				HasTexCoords ? MeshData.TexCoords[VertexIndex] : FVector2D::ZeroVector;

			const FVector TangentX =
				HasTangents ? MeshData.Tangents[VertexIndex] : FVector::ZeroVector;
			const FVector TangentZ = MeshData.Normals[VertexIndex];
			const FVector TangentY =
				HasTangents ? (TangentZ ^ TangentX).GetSafeNormal() : FVector::ZeroVector;

			VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex) = VertexPosition;
			VertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex) = VertexColor;
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertexIndex, 0, VertexTexCoord);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				VertexIndex, TangentX, TangentY, TangentZ);

			// Populate Index Buffer Resource
			if (!HasIndexBuffer)
			{
				IndexBuffer.Indices[VertexIndex] = VertexIndex;
			}
		}

		// Populate Index Buffer Resource
		if (HasIndexBuffer)
		{
			for (int32 Index = 0; Index < NumIndices; ++Index)
			{
				IndexBuffer.Indices[Index] = MeshData.Indices[Index];
			}
		}

		// Generate Tangents and Binormals
		if (!HasTangents)
		{
			for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles; ++TriangleIndex)
			{
				// Compute Triangle Tangent.

				const uint32& VertexIndex0 = IndexBuffer.Indices[TriangleIndex * 3 + 0];
				const uint32& VertexIndex1 = IndexBuffer.Indices[TriangleIndex * 3 + 1];
				const uint32& VertexIndex2 = IndexBuffer.Indices[TriangleIndex * 3 + 2];

				const FVector& Position0 = MeshData.Vertices[VertexIndex0];
				const FVector& Position1 = MeshData.Vertices[VertexIndex1];
				const FVector& Position2 = MeshData.Vertices[VertexIndex2];

				const FVector P0toP1 = Position1 - Position0;
				const FVector P0toP2 = Position2 - Position0;

				FVector TriangleTangent;

				if (HasTexCoords)
				{
					const FVector2D& TexCoord0 = MeshData.TexCoords[VertexIndex0];
					const FVector2D& TexCoord1 = MeshData.TexCoords[VertexIndex1];
					const FVector2D& TexCoord2 = MeshData.TexCoords[VertexIndex2];

					const float U0toU1 = TexCoord1.X - TexCoord0.X;
					const float U0toU2 = TexCoord2.X - TexCoord0.X;

					const float V0toV1 = TexCoord1.Y - TexCoord0.Y;
					const float V0toV2 = TexCoord2.Y - TexCoord0.Y;

					TriangleTangent = FVector(
						V0toV2 * P0toP1.X - V0toV1 * P0toP2.X,
						V0toV2 * P0toP1.Y - V0toV1 * P0toP2.Y,
						V0toV2 * P0toP1.Z - V0toV1 * P0toP2.Z);
				}
				else
				{
					TriangleTangent = P0toP1;
				}

				// Compute Tangent and Binormal for each vertex of the triangle, so that
				// potentially smoothed normals produces smooth Tangents and Binormals.
				for (uint32 VertexIndexInTriangle = 0; VertexIndexInTriangle < 3;
					 ++VertexIndexInTriangle)
				{
					const uint32& VertexIndex =
						IndexBuffer.Indices[TriangleIndex * 3 + VertexIndexInTriangle];
					const FVector& VertexNormal = MeshData.Normals[VertexIndex];
					const FVector VertexBinormal = (VertexNormal ^ TriangleTangent).GetSafeNormal();
					const FVector VertexTangent = (VertexBinormal ^ VertexNormal).GetSafeNormal();

					VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
						VertexIndex, VertexTangent, VertexBinormal, VertexNormal);
				}
			}
		}

		// Enqueue initialization of render resource
		ENQUEUE_RENDER_COMMAND(FAGX_SimpleMeshSceneProxyVertexBuffersInit)
		([this, LightMapIndex](FRHICommandListImmediate& RHICmdList) {
			VertexBuffers.PositionVertexBuffer.InitResource();
			VertexBuffers.StaticMeshVertexBuffer.InitResource();
			VertexBuffers.ColorVertexBuffer.InitResource();

			FLocalVertexFactory::FDataType Data;
			VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(
				&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(
				&VertexFactory, Data, LightMapIndex);
			VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&VertexFactory, Data);
			VertexFactory.SetData(Data);

			VertexFactory.InitResource();
			IndexBuffer.InitResource();
		});

		// Grab material
		Material = Component->GetMaterial(0);
		if (Material == NULL)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	virtual ~FAGX_SimpleMeshSceneProxy()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_AGX_SimpleMeshSceneProxy_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f));

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if (bWireframe)
		{
			MaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy();
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				// Draw the mesh.
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;

				bool bHasPrecomputedVolumetricLightmap;
				FMatrix PreviousLocalToWorld;
				int32 SingleCaptureIndex;
#if UE_VERSION_OLDER_THAN(4, 23, 0)
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
					GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap,
					PreviousLocalToWorld, SingleCaptureIndex);
#else
				/// \todo Replace Unknown with proper name or use some getter function to get a
				/// proper value.
				bool Unknown = false;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
					GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap,
					PreviousLocalToWorld, SingleCaptureIndex, Unknown);
#endif
				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer =
					Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
#if UE_VERSION_OLDER_THAN(4, 23, 0)
				DynamicPrimitiveUniformBuffer.Set(
					GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true,
					bHasPrecomputedVolumetricLightmap, UseEditorDepthTest());
#else
				DynamicPrimitiveUniformBuffer.Set(
					GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true,
					bHasPrecomputedVolumetricLightmap, DrawsVelocity(), Unknown);
#endif
				BatchElement.PrimitiveUniformBufferResource =
					&DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex =
					VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		Result.bVelocityRelevance = IsMovable() &&
#if UE_VERSION_OLDER_THAN(4, 25, 0)
									Result.bOpaqueRelevance &&
#else
									Result.bOpaque &&
#endif
									Result.bRenderInMainPass;
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const override
	{
		return (sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return (FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	UMaterialInterface* Material;
	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
	FLocalVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
};

//////////////////////////////////////////////////////////////////////////

UAGX_SimpleMeshComponent::UAGX_SimpleMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

bool UAGX_SimpleMeshComponent::SetMeshData(const TSharedPtr<FAGX_SimpleMeshData>& Data)
{
	MeshData = Data;

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
	UpdateBounds();

	return true;
}

void UAGX_SimpleMeshComponent::ClearMeshData()
{
	MeshData.Reset();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
	UpdateBounds();
}

FPrimitiveSceneProxy* UAGX_SimpleMeshComponent::CreateSceneProxy()
{
	FPrimitiveSceneProxy* Proxy = NULL;
	if (MeshData && MeshData->Vertices.Num() > 0 /*&& MeshData->IsValid()*/)
	{
		Proxy = new FAGX_SimpleMeshSceneProxy(this);
	}
	return Proxy;
}

int32 UAGX_SimpleMeshComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UAGX_SimpleMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox BoundingBox(ForceInit);

	// Bounds are tighter if the box is generated from pre-transformed vertices.
	if (MeshData /*&& MeshData->IsValid()*/)
	{
		for (int32 Index = 0; Index < MeshData->Vertices.Num(); ++Index)
		{
			BoundingBox += LocalToWorld.TransformPosition(MeshData->Vertices[Index]);
		}
	}

	FBoxSphereBounds NewBounds;
	NewBounds.BoxExtent = BoundingBox.GetExtent();
	NewBounds.Origin = BoundingBox.GetCenter();
	NewBounds.SphereRadius = NewBounds.BoxExtent.Size();

	return NewBounds;
}
