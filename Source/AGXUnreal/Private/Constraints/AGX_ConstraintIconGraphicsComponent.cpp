/// \todo Reduce includes!
#include "AGX_ConstraintIconGraphicsComponent.h"

#include <optional>

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
#include "UObject/ConstructorHelpers.h"

#include "AGX_MeshUtilities.h"
#include "Constraints/AGX_BallConstraint.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Constraints/AGX_CylindricalConstraint.h"
#include "Constraints/AGX_DistanceConstraint.h"
#include "Constraints/AGX_HingeConstraint.h"
#include "Constraints/AGX_LockConstraint.h"
#include "Constraints/AGX_PrismaticConstraint.h"

/**
 * Holds vertex and index buffers for rendering.
 */
struct FAGX_ConstraintIconGraphicsGeometry
{
	EPrimitiveType Type = PT_TriangleList;

	FStaticMeshVertexBuffers VertexBuffers;
	FLocalVertexFactory VertexFactory;
	FDynamicMeshIndexBuffer32 IndexBuffer;

	FAGX_ConstraintIconGraphicsGeometry(EPrimitiveType InType, ERHIFeatureLevel::Type FeatureLevel, const char* BuffersDebugName)
		: Type(InType), VertexFactory(FeatureLevel, BuffersDebugName)
	{ }

	uint32 NumIndexesPerPrimitive() const
	{
		return NumIndexesPerPrimitive(Type);
	}

	static uint32 NumIndexesPerPrimitive(EPrimitiveType Type)
	{
		switch (Type)
		{
		case PT_TriangleList:
			return 3;
		case PT_LineList:
			return 2;
		default:
			check(!"FAGX_ConstraintIconGraphicsGeometry does not support this primitive type!");
			return 1;
		}
	}
};

/**
 * Defines rendering of a section of a geometry, with a specific material and transformation.
 * Sections can be used to for example render different parts of the geometry with different materials,
 * or to render the same geometry multiple times with different transformations and materials.
 */
struct FAGX_ConstraintIconGraphicsSection
{
	TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry;
	uint32 BeginIndex; // First index to render in the geometry's index buffer.
	uint32 EndIndex; // One past last index to render in geometry's index buffer.
	UMaterialInterface* Material = nullptr;
	bool bShowSelectionOutline;
	FMatrix LocalTransform = FMatrix::Identity;
	ESceneDepthPriorityGroup DepthPriority;


	FAGX_ConstraintIconGraphicsSection(const TSharedPtr<FAGX_ConstraintIconGraphicsGeometry>& InGeometry,
		UMaterialInterface* InMaterial, bool bInShowSelectionOutline, FMatrix InLocalTransform,
		ESceneDepthPriorityGroup InDepthPriority, uint32 InBeginIndex = 0, int32 InEndIndex = -1)
		:
	Geometry(InGeometry),
	BeginIndex(InBeginIndex),
	EndIndex(InEndIndex >= 0 ? InEndIndex : (InGeometry ? static_cast<uint32>(InGeometry->IndexBuffer.Indices.Num()) : 0)),
	Material(InMaterial),
	bShowSelectionOutline(bInShowSelectionOutline),
	LocalTransform(InLocalTransform),
	DepthPriority(InDepthPriority)
	{ }

	uint32 GetNumPrimitives() const
	{
		return Geometry ? (EndIndex - BeginIndex) / Geometry->NumIndexesPerPrimitive() : 0;
	}

	uint32 GetMinVertexIndex() const
	{
		return 0;
	}

	uint32 GetMaxVertexIndex() const
	{
		return Geometry ? Geometry->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1 : 0;
	}
};

/**
 * Render proxy for FAGX_ConstraintIconGraphics. Handles render resources. Accessed by both game and render thread.
 */
class FAGX_ConstraintIconGraphicsProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FAGX_ConstraintIconGraphicsProxy(UAGX_ConstraintIconGraphicsComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel())),
		bDrawOnlyIfUnselected(true),
		LockedDofs(Component->Constraint->GetLockedDofsBitmask()),
		FrameTransform1(Component->Constraint->BodyAttachment1.GetGlobalFrameMatrix()),
		FrameTransform2(Component->Constraint->BodyAttachment2.GetGlobalFrameMatrix())
	{
		/// \todo Use inheritance instead of this branching below.
		/// \todo IsA() should probably not be used if future constraints will derive these spawnable constraints.
		if (Component->GetOwner()->IsA(AAGX_BallConstraint::StaticClass()))
		{
			CreateBallConstraint(Component);
		}
		else if (Component->GetOwner()->IsA(AAGX_CylindricalConstraint::StaticClass()))
		{
			CreateCylindricalConstraint(Component);
		}
		else if (Component->GetOwner()->IsA(AAGX_DistanceConstraint::StaticClass()))
		{
			/// \todo
		}
		else if (Component->GetOwner()->IsA(AAGX_HingeConstraint::StaticClass()))
		{
			/// \todo
		}
		else if (Component->GetOwner()->IsA(AAGX_LockConstraint::StaticClass()))
		{
			/// \todo
		}
		else if (Component->GetOwner()->IsA(AAGX_PrismaticConstraint::StaticClass()))
		{
			CreateCylindricalConstraint(Component, 4); // Using low-res cylindrical for boxy look. \todo Make dedicated!
		}

		// Enqueue initialization of render resource
		for (const TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry : Geometries)
		{
			ENQUEUE_RENDER_COMMAND(FAGX_ConstraintIconGraphicsVertexBuffersInit)(
				[Geometry = Geometry.Get()](FRHICommandListImmediate& RHICmdList)
			{
				Geometry->VertexBuffers.PositionVertexBuffer.InitResource();
				Geometry->VertexBuffers.StaticMeshVertexBuffer.InitResource();
				Geometry->VertexBuffers.ColorVertexBuffer.InitResource();

				FLocalVertexFactory::FDataType Data;
				Geometry->VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(&Geometry->VertexFactory, Data, /*LightMapIndex*/ 0);
				Geometry->VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexFactory.SetData(Data);

				Geometry->VertexFactory.InitResource();
				Geometry->IndexBuffer.InitResource();
			});

			UE_LOG(LogTemp, Log, TEXT("FAGX_ConstraintIconGraphicsProxy for \"%s\""), *GetNameSafe(Component->GetOwner()));
			AGX_MeshUtilities::PrintMeshToLog(Geometry->VertexBuffers, Geometry->IndexBuffer);
		};
	}

	virtual ~FAGX_ConstraintIconGraphicsProxy()
	{
		for (const TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry : Geometries)
		{
			Geometry->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Geometry->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Geometry->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Geometry->IndexBuffer.ReleaseResource();
			Geometry->VertexFactory.ReleaseResource();
		}
	}

	void SetAttachmentFrameTransforms(const FMatrix& InFrameTransform1, const FMatrix& InFrameTransform2)
	{
		FrameTransform1 = InFrameTransform1;
		FrameTransform2 = InFrameTransform2;
	}

private:

	bool IsDofLocked(EDofFlag Dof)
	{
		return static_cast<uint8>(LockedDofs) & static_cast<uint8>(Dof);
	}

	bool GetShowSelectionOutline(EDofFlag Dof)
	{
		return false; // looks best without it
		//return !IsDofLocked(Dof);
	}

	ESceneDepthPriorityGroup GetDepthPriority()
	{
		return SDPG_Foreground;
		//return SDPG_World; /// \note Also need to turn off "Disable Depth Test" on material for it to be cullable.
	}

	void CreateBallConstraint(UAGX_ConstraintIconGraphicsComponent* Component)
	{
		// Create the geometry.

		TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry = MakeShared<FAGX_ConstraintIconGraphicsGeometry>(
			PT_TriangleList, GetScene().GetFeatureLevel(), "FAGX_ConstraintIconGraphicsGeometry");

		// Create geometry definition.

		const float Radius = 20.0f;
		const uint32 NumSegments = 24;

		const FLinearColor Opaque(1, 1, 1, 1);
		const FLinearColor SemiTransparent(1, 1, 1, 0.5);

		AGX_MeshUtilities::DiskArrayConstructionData DiskArrayData(Radius, NumSegments, 0.0f, 3, true, SemiTransparent, SemiTransparent,
			{ FTransform(FRotator(90, 0, 0)), FTransform(FRotator(0, 90, 0)), FTransform(FRotator(0, 0, 90)) });

		AGX_MeshUtilities::SphereConstructionData SphereData(Radius, NumSegments);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		DiskArrayData.AppendBufferSizes(NumVertices, NumIndices);
		SphereData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Create Sections and Fill Geometry buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(Geometry, Component->GetOuterShellMaterial(),
				true, FMatrix::Identity, GetDepthPriority(), NumAddedIndices, NumAddedIndices + DiskArrayData.Indices));

			AGX_MeshUtilities::MakeDiskArray(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
				NumAddedIndices, DiskArrayData);
		}

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(Geometry, Component->GetInnerShellMaterial(),
				true, FMatrix::Identity, GetDepthPriority(), NumAddedIndices, NumAddedIndices + SphereData.Indices));

			AGX_MeshUtilities::MakeSphere(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
				NumAddedIndices, SphereData);
		}

		Geometries.Add(Geometry);
	}

	void CreateCylindricalConstraint(UAGX_ConstraintIconGraphicsComponent* Component, uint32 NumSegments = 16)
	{
		// Create the geometry.

		TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry = MakeShared<FAGX_ConstraintIconGraphicsGeometry>(
			PT_TriangleList, GetScene().GetFeatureLevel(), "FAGX_ConstraintIconGraphicsGeometry");

		// Create geometry definition.

		const float InnerRadius = 9.0f;
		const float OuterRadius = 16.0f;
		const FLinearColor Opaque(1, 1, 1, 1);
		const FLinearColor Transparent(1, 1, 1, 0);

		AGX_MeshUtilities::DiskArrayConstructionData DiskArrayData(InnerRadius, NumSegments, 5.0f, 18, true, Opaque, Transparent);
		AGX_MeshUtilities::CylinderConstructionData InnerCylinderData(InnerRadius, 110.0f, NumSegments, 2, Opaque, Transparent);
		AGX_MeshUtilities::CylinderConstructionData OuterCylinderData(OuterRadius, 50.0f, NumSegments, 1, Opaque, Opaque);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		DiskArrayData.AppendBufferSizes(NumVertices, NumIndices);
		InnerCylinderData.AppendBufferSizes(NumVertices, NumIndices);
		OuterCylinderData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Create Sections and Fill Geometry buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(Geometry, Component->GetInnerDisksMaterial(),
				true, FMatrix::Identity, GetDepthPriority(), NumAddedIndices, NumAddedIndices + DiskArrayData.Indices));

			AGX_MeshUtilities::MakeDiskArray(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
				NumAddedIndices, DiskArrayData);
		}

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(Geometry, Component->GetInnerShellMaterial(),
				true, FMatrix::Identity, GetDepthPriority(), NumAddedIndices, NumAddedIndices + InnerCylinderData.Indices));

			AGX_MeshUtilities::MakeCylinder(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
				NumAddedIndices, InnerCylinderData);
		}

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(Geometry, Component->GetOuterShellMaterial(),
				true, FMatrix::Identity, GetDepthPriority(), NumAddedIndices, NumAddedIndices + OuterCylinderData.Indices));

			AGX_MeshUtilities::MakeCylinder(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
				NumAddedIndices, OuterCylinderData);
		}

		Geometries.Add(Geometry);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_AGX_ConstraintIconGraphics_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f));

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		for (const TSharedPtr<FAGX_ConstraintIconGraphicsSection> Section : Sections)
		{
			FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					FMeshBatch& Mesh = Collector.AllocateMesh();
					Mesh.bWireframe = bWireframe;
					Mesh.MaterialRenderProxy = MaterialProxy;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.bCanApplyViewModeOverrides = false;
					Mesh.VertexFactory = &Section->Geometry->VertexFactory;
					Mesh.bSelectable = true;
					Mesh.Type = Section->Geometry->Type;
					Mesh.bUseSelectionOutline = Section->bShowSelectionOutline;
					Mesh.DepthPriorityGroup = Section->DepthPriority;

					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &Section->Geometry->IndexBuffer;
					BatchElement.FirstIndex = Section->BeginIndex;
					BatchElement.NumPrimitives = Section->GetNumPrimitives();
					BatchElement.MinVertexIndex = Section->GetMinVertexIndex();
					BatchElement.MaxVertexIndex = Section->GetMaxVertexIndex();

					FMatrix WorldMatrix = GetLocalToWorld().GetMatrixWithoutScale();
					//FMatrix WorldMatrix = FrameTransform1; // setting render matrix instead (see GetRenderMatrix())

					FMatrix ScreenScale = GetScreenSpaceScale(0.2f, 30.0f, 100.0f, 100.0f,
						WorldMatrix.GetOrigin(), View);

					/// todo ScreenScale does not seem to have effect in-game. Must have missed something...

					FMatrix EffectiveLocalToWorld = Section->LocalTransform * ScreenScale * WorldMatrix;

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer =
						Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();

					DynamicPrimitiveUniformBuffer.Set(EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(),
						GetLocalBounds(), true, false, UseEditorDepthTest());

					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}

	/// Will create a matrix that scales an object of original size 'OriginalWorldSize' to occupy the desired
	/// 'NormalizedScreenSpaceSize' fraction of the screen horizontally, but limiting it within MinWorldSize and
	/// MaxWorldSize. Result is not 100% correct, but it's consistent when moving around, so just tweak input!
	static FMatrix GetScreenSpaceScale(float NormalizedScreenSpaceSize, float MinWorldSize, float MaxWorldSize,
		float OriginalWorldSize, const FVector& WorldLocation, const FSceneView *View)
	{
		float Distance = (WorldLocation - View->ViewLocation).Size();
		float NormalizedScreenToWorld = 2.0f * Distance * FMath::Atan(FMath::DegreesToRadians(View->FOV) / 2.0f);
		float WorldSize = FMath::Clamp(NormalizedScreenSpaceSize * NormalizedScreenToWorld, MinWorldSize, MaxWorldSize);
		float Scale = WorldSize / OriginalWorldSize;

		return FScaleMatrix(Scale);
	}

	UMaterialInterface* GetSectionMaterial(UAGX_ConstraintIconGraphicsComponent* Component, uint32 SectionIndex)
	{
		if (UMaterialInterface* Material = Component->GetMaterial(SectionIndex))
		{
			return Material;
		}
		else
		{
			return UMaterial::GetDefaultMaterial(MD_Surface);
		}
	};

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		const bool bVisibleForSelection = !bDrawOnlyIfUnselected || !IsSelected();

		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && bVisibleForSelection;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		Result.bVelocityRelevance = IsMovable() && Result.bOpaqueRelevance && Result.bRenderInMainPass;

		MaterialRelevance.SetPrimitiveViewRelevance(Result);

		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual bool IsUsingDistanceCullFade() const override
	{
		return MaterialRelevance.bUsesDistanceCullFade;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:

	TArray<TSharedPtr<FAGX_ConstraintIconGraphicsGeometry>> Geometries;
	TArray<TSharedPtr<FAGX_ConstraintIconGraphicsSection>> Sections;
	FMaterialRelevance MaterialRelevance;
	const bool bDrawOnlyIfUnselected = true;
	const EDofFlag LockedDofs;
	FMatrix FrameTransform1; // global transforms
	FMatrix FrameTransform2;
};

UAGX_ConstraintIconGraphicsComponent::UAGX_ConstraintIconGraphicsComponent(const FObjectInitializer& ObjectInitializer)
	:
Super(ObjectInitializer),
OuterShellMaterialIndex(-1),
InnerShellMaterialIndex(-1),
InnerDisksMaterialIndex(-1)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	bCastHiddenShadow = false;

	if (AActor* Owner = GetOwner())
	{
		// Enabling the SceneComponents's white blob for constraint types with no visualization yet.
		/// \todo Set bVisualizeComponent to false when these constraints' graphics are implemented by in the proxy.
		bVisualizeComponent = Owner->IsA(AAGX_DistanceConstraint::StaticClass()) ||
			Owner->IsA(AAGX_HingeConstraint::StaticClass()) || Owner->IsA(AAGX_LockConstraint::StaticClass());
	}


	// Find materials.
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> OuterShellMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintIconGraphics_OuterShell"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> InnerShellMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintIconGraphics_InnerShell"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> InnerDisksMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintIconGraphics_InnerDisks"));

		UMaterialInterface* FallbackMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

		UMaterialInterface* OuterShellMaterial = OuterShellMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(OuterShellMaterialFinder.Object) : FallbackMaterial;

		UMaterialInterface* InnerShellMaterial = InnerShellMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(InnerShellMaterialFinder.Object) : FallbackMaterial;

		UMaterialInterface* InnerDisksMaterial = InnerDisksMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(InnerDisksMaterialFinder.Object) : FallbackMaterial;

		OuterShellMaterialIndex = GetNumMaterials();
		SetMaterial(OuterShellMaterialIndex, OuterShellMaterial);

		InnerShellMaterialIndex = GetNumMaterials();
		SetMaterial(InnerShellMaterialIndex, InnerShellMaterial);

		InnerDisksMaterialIndex = GetNumMaterials();
		SetMaterial(InnerDisksMaterialIndex, InnerDisksMaterial);
	}
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetOuterShellMaterial() const
{
	return GetMaterial(OuterShellMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetInnerShellMaterial() const
{
	return GetMaterial(InnerShellMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetInnerDisksMaterial() const
{
	return GetMaterial(InnerDisksMaterialIndex);
}

void UAGX_ConstraintIconGraphicsComponent::OnBecameSelected()
{
	MarkRenderTransformDirty();
	MarkRenderDynamicDataDirty();
}

FPrimitiveSceneProxy* UAGX_ConstraintIconGraphicsComponent::CreateSceneProxy()
{
	if(!Constraint)
	{
		return nullptr;
	}
	MarkRenderDynamicDataDirty();
	return new FAGX_ConstraintIconGraphicsProxy(this);
}

int32 UAGX_ConstraintIconGraphicsComponent::GetNumMaterials() const
{
	return GetNumOverrideMaterials();
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetMaterial(int32 ElementIndex) const
{
	return Super::GetMaterial(ElementIndex);
}

void UAGX_ConstraintIconGraphicsComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	return Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
}

FBoxSphereBounds UAGX_ConstraintIconGraphicsComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	/// \todo Make more precise!

	FBoxSphereBounds NewBounds;
	NewBounds.BoxExtent = FVector(100.0f, 100.0f, 100.0f);
	NewBounds.Origin = LocalToWorld.GetLocation();
	NewBounds.SphereRadius = 150.0f;

	return NewBounds;
}

void UAGX_ConstraintIconGraphicsComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	/// \note Not using this data anymore. Instead we set render matrix directly using the frame (see GetRenderMatrix).

	// Update transform of the proxy to match the constraint attachment frame, if out-of-date!
	if (SceneProxy && Constraint && IsOwnerSelected())
	{
		FMatrix Frame1 = Constraint->BodyAttachment1.GetGlobalFrameMatrix();
		FMatrix Frame2 = Constraint->BodyAttachment2.GetGlobalFrameMatrix();

		FAGX_ConstraintIconGraphicsProxy* CastProxy = static_cast<FAGX_ConstraintIconGraphicsProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FSendConstraintIconGraphicsDynamicData)(
			[CastProxy, Frame1, Frame2](FRHICommandListImmediate& RHICmdList)
			{
				CastProxy->SetAttachmentFrameTransforms(Frame1, Frame2);
			});
	}
}

FMatrix UAGX_ConstraintIconGraphicsComponent::GetRenderMatrix() const
{
	if (Constraint)
	{
		return Constraint->BodyAttachment1.GetGlobalFrameMatrix();
	}
	else
	{
		return FMatrix::Identity;
	}
}
