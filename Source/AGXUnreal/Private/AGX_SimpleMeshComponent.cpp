// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved. 

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
		const FColor VertexColor(255,255,255);
		const int32 NumTris = Component->MeshTris.Num();
		const int32 NumVertices = NumTris * 3;
		uint32 NumTexCoords = 1;
		uint32 LightMapIndex = 0;

		IndexBuffer.Indices.AddUninitialized(NumVertices);

		check(NumTexCoords < MAX_STATIC_TEXCOORDS && NumTexCoords > 0);
		check(LightMapIndex < NumTexCoords);

		if (NumTris)
		{
			VertexBuffers.PositionVertexBuffer.Init(NumVertices);
			VertexBuffers.StaticMeshVertexBuffer.Init(NumVertices, NumTexCoords);
			VertexBuffers.ColorVertexBuffer.Init(NumVertices);

			for (int32 TriIndex = 0; TriIndex < NumTris; ++TriIndex)
			{
				FAGX_SimpleMeshTriangle& Tri = Component->MeshTris[TriIndex];

				const FVector Edge01 = (Tri.Vertex1 - Tri.Vertex0);
				const FVector Edge02 = (Tri.Vertex2 - Tri.Vertex0);

				const FVector TangentX = Edge01.GetSafeNormal();
				const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
				const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

				for (int32 TriVertIndex = 0; TriVertIndex < 3; ++TriVertIndex)
				{
					uint32 VertexIndex = TriIndex * 3 + TriVertIndex;

					VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex) = Tri[TriVertIndex];
					VertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex) = VertexColor;
					VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(VertexIndex, TangentX, TangentY, TangentZ);

					for (uint32 TexCoordIndex = 0; TexCoordIndex < NumTexCoords; ++TexCoordIndex)
					{
						VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertexIndex, TexCoordIndex, FVector2D::ZeroVector);
					}

					IndexBuffer.Indices[VertexIndex] = VertexIndex;
				}
			}
		}
		else
		{
			VertexBuffers.PositionVertexBuffer.Init(1);
			VertexBuffers.StaticMeshVertexBuffer.Init(1, 1);
			VertexBuffers.ColorVertexBuffer.Init(1);

			VertexBuffers.PositionVertexBuffer.VertexPosition(0) = FVector::ZeroVector;
			VertexBuffers.ColorVertexBuffer.VertexColor(0) = FColor(1, 1, 1, 1);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(0, FVector::ForwardVector, FVector::RightVector, FVector::UpVector);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(0, 0, FVector2D::ZeroVector);

			NumTexCoords = 1;
			LightMapIndex = 0;
		}

		// Enqueue initialization of render resource
		ENQUEUE_RENDER_COMMAND(FAGX_SimpleMeshSceneProxyVertexBuffersInit)(
			[this, LightMapIndex](FRHICommandListImmediate& RHICmdList)
		{
			VertexBuffers.PositionVertexBuffer.InitResource();
			VertexBuffers.StaticMeshVertexBuffer.InitResource();
			VertexBuffers.ColorVertexBuffer.InitResource();

			FLocalVertexFactory::FDataType Data;
			VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(&VertexFactory, Data, LightMapIndex);
			VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&VertexFactory, Data);
			VertexFactory.SetData(Data);

			VertexFactory.InitResource();
			IndexBuffer.InitResource();
		});

		// Grab material
		Material = Component->GetMaterial(0);
		if(Material == NULL)
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

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_AGX_SimpleMeshSceneProxy_GetDynamicMeshElements );

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if(bWireframe)
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
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex);

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, UseEditorDepthTest());
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
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
		Result.bVelocityRelevance = IsMovable() && Result.bOpaqueRelevance && Result.bRenderInMainPass;
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint( void ) const override { return( sizeof( *this ) + GetAllocatedSize() ); }

	uint32 GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

private:

	UMaterialInterface* Material;
	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
	FLocalVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
};

//////////////////////////////////////////////////////////////////////////

UAGX_SimpleMeshComponent::UAGX_SimpleMeshComponent( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

bool UAGX_SimpleMeshComponent::SetMeshTriangles(const TArray<FAGX_SimpleMeshTriangle>& Triangles)
{
	MeshTris = Triangles;

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
	UpdateBounds();

	return true;
}

void UAGX_SimpleMeshComponent::AddMeshTriangles(const TArray<FAGX_SimpleMeshTriangle>& Triangles)
{
	MeshTris.Append(Triangles);

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
	UpdateBounds();
}

void  UAGX_SimpleMeshComponent::ClearMeshTriangles()
{
	MeshTris.Reset();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
	UpdateBounds();
}


FPrimitiveSceneProxy* UAGX_SimpleMeshComponent::CreateSceneProxy()
{
	FPrimitiveSceneProxy* Proxy = NULL;
	if(MeshTris.Num() > 0)
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
	for (int32 Index = 0; Index < MeshTris.Num(); ++Index)
	{
		BoundingBox += LocalToWorld.TransformPosition(MeshTris[Index].Vertex0);
		BoundingBox += LocalToWorld.TransformPosition(MeshTris[Index].Vertex1);
		BoundingBox += LocalToWorld.TransformPosition(MeshTris[Index].Vertex2);
	}

	FBoxSphereBounds NewBounds;
	NewBounds.BoxExtent = BoundingBox.GetExtent();
	NewBounds.Origin = BoundingBox.GetCenter();
	NewBounds.SphereRadius = NewBounds.BoxExtent.Size();

	return NewBounds;
}

