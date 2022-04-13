// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_EditorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterialAsset.h"

// Unreal Engine includes.
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Editor/EditorEngine.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameEngine.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Char.h"
#include "Misc/EngineVersionComparison.h"
#include "RawMesh.h"
#include "UObject/SavePackage.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FAGX_EditorUtilities"

std::tuple<AActor*, USceneComponent*> FAGX_EditorUtilities::CreateEmptyActor(
	const FTransform& Transform, UWorld* World)
{
	/// \todo The intention is to mimmic dragging in an "Empty Actor" from the
	/// Place mode. Investigate if we can use ActorFactoryEmptyActor instead.

	AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass());
	if (NewActor == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Failed to create empty actor."));
		/// \todo Do we need to destroy the Actor here?
		return {nullptr, nullptr};
	}

	/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
	/// Related to undo/redo, I think.
	USceneComponent* Root = NewObject<USceneComponent>(
		NewActor, USceneComponent::GetDefaultSceneRootVariableName() /*, RF_Transactional*/);
	NewActor->SetRootComponent(Root);
	NewActor->AddInstanceComponent(Root);
	Root->RegisterComponent();
	NewActor->SetActorTransform(Transform, false);

	return {NewActor, Root};
}

namespace
{
	template <typename TComponent>
	TComponent* CreateComponent(AActor* Owner)
	{
		UClass* Class = TComponent::StaticClass();
		TComponent* Component = NewObject<TComponent>(Owner, Class);
		if (Component == nullptr)
		{
			UE_LOG(LogAGX, Log, TEXT("Could not create component %s."), *Class->GetName());
			return nullptr;
		}
		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	template <typename TShapeComponent>
	TShapeComponent* CreateShapeComponent(AActor* Owner, USceneComponent* Outer, bool bRegister)
	{
		/// \todo Is the Owner pointless here since we do `AttachToComponent`
		/// immediately afterwards?
		UClass* Class = TShapeComponent::StaticClass();
		TShapeComponent* Shape = NewObject<TShapeComponent>(Owner, Class);
		Owner->AddInstanceComponent(Shape);
		if (bRegister)
		{
			Shape->RegisterComponent();
		}
		if (Outer)
		{
			const bool Attached = Shape->AttachToComponent(
				Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			check(Attached);
		}
		return Shape;
	}

	template <typename T>
	static T* GetAssetByPath(const FString& AssetPath)
	{
		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;
		FARFilter Filter;
		Filter.PackageNames.Add(FName(*AssetPath));
		AssetRegistryModule.Get().GetAssets(Filter, AssetData);
		T* Asset = FAssetData::GetFirstAsset<T>(AssetData);

		if (Asset == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Could not find asset with path %s."), *AssetPath);
		}

		return Asset;
	}
}

UAGX_RigidBodyComponent* FAGX_EditorUtilities::CreateRigidBody(AActor* Owner)
{
	UAGX_RigidBodyComponent* Body = ::CreateComponent<UAGX_RigidBodyComponent>(Owner);
	Body->AttachToComponent(
		Owner->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
#if UE_VERSION_OLDER_THAN(4, 24, 0)
	Body->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
	Body->RelativeRotation = FRotator(0.0f, 0.0f, 0.0f);
#else
	Body->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Body->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
#endif
	return Body;
}

/// \todo Can the Owner parameter be removed, and instead use Outer->GetOwner()?
/// When would we want to attach the sphere to a component that is in another Actor.
UAGX_SphereShapeComponent* FAGX_EditorUtilities::CreateSphereShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_SphereShapeComponent>(Owner, Outer, true);
}

UAGX_BoxShapeComponent* FAGX_EditorUtilities::CreateBoxShape(AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_BoxShapeComponent>(Owner, Outer, true);
}

UAGX_CylinderShapeComponent* FAGX_EditorUtilities::CreateCylinderShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CylinderShapeComponent>(Owner, Outer, true);
}

UAGX_CapsuleShapeComponent* FAGX_EditorUtilities::CreateCapsuleShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CapsuleShapeComponent>(Owner, Outer, true);
}

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(
	AActor* Owner, USceneComponent* Outer, bool bRegister)
{
	return ::CreateShapeComponent<UAGX_TrimeshShapeComponent>(Owner, Outer, bRegister);
}

/// \todo There is probably a name sanitizer already in Unreal. Find it.
/// \todo The sanitizers are called multiple times on the same string. Find a root function, or
/// a suitable helper function, and sanitize once. Assume already sanitizied in all other helper
/// functions.

FString FAGX_EditorUtilities::SanitizeName(const FString& Name)
{
	FString Sanitized;
	Sanitized.Reserve(Name.Len());
	for (TCHAR C : Name)
	{
		/// \todo Will this accept non-english characters? Should it?
		if (TChar<TCHAR>::IsAlnum(C) || C == TCHAR('_'))
		{
			Sanitized.AppendChar(C);
		}
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::SanitizeName(const FString& Name, const FString& Fallback)
{
	FString Sanitized = SanitizeName(Name);
	if (Sanitized.IsEmpty())
	{
		return Fallback;
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::SanitizeName(const FString& Name, const TCHAR* Fallback)
{
	FString Sanitized = SanitizeName(Name);
	if (Sanitized.IsEmpty())
	{
		return FString(Fallback);
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::CreateAssetName(
	FString SourceName, FString ActorName, FString DefaultName)
{
	SourceName = FAGX_EditorUtilities::SanitizeName(SourceName);
	if (!SourceName.IsEmpty())
	{
		return SourceName;
	}

	ActorName = FAGX_EditorUtilities::SanitizeName(ActorName);
	if (!ActorName.IsEmpty())
	{
		return ActorName;
	}

	DefaultName = FAGX_EditorUtilities::SanitizeName(DefaultName);
	if (!DefaultName.IsEmpty())
	{
		return DefaultName;
	}

	return TEXT("ImportedAgxObject");
}

void FAGX_EditorUtilities::MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName)
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackageName, AssetName, PackageName, AssetName);
}

bool FAGX_EditorUtilities::FinalizeAndSavePackage(
	UPackage* Package, UObject* Asset, const FString& PackagePath, const FString& AssetName)
{
	/// \todo Can the PackagePath and AssetName be read from the Package and Asset? To reduce
	/// the number of parameters and avoid passing mismatching arguments. When would we want
	/// to custom PackagePath and AssetName?

	FAssetRegistryModule::AssetCreated(Asset);
	Asset->MarkPackageDirty();
	Asset->PostEditChange();
	Asset->AddToRoot();
	Package->SetDirtyFlag(true);

	// Store our new package to disk.
	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		PackagePath, FPackageName::GetAssetPackageExtension());
	if (PackageFilename.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unreal Engine unable to provide a package filename for package path '%s'."),
			*PackagePath);
		return false;
	}

	// A package must have meta-data in order to be saved. It seems to be created automatically
	// most of the time but sometimes, during unit tests for example, the engine tries to create it
	// on-demand while saving the package which leads to a fatal error because this type of object
	// look-up isn't allowed while saving packages. So try to force it here before calling
	// SavePackage.
	//
	// The error message sometimes printed while within UPackage::SavePackage called below is:
	// Illegal call to StaticFindObjectFast() while serializing object data or garbage collecting!
	Package->GetMetaData();
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	bool bSaved = UPackage::SavePackage(Package, Asset, RF_NoFlags, *PackageFilename);
#else
	FSavePackageArgs SaveArgs;
	// SaveArgs.TargetPlatform = ???; // I think we can leave this at the default: not cooking.
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	// SaveArgs.SaveFlags = ???; // I think we can leave this at the default: None.
	// SaveArgs.bForceByteSwapping = ???; // I think we can leave this at the default: false.
	// SaveArgs.bWarnOfLongFilename = ???; // I think we can leave this at the default: true.
	// SaveArgs.bSlowTask = ???; // I think we can leave this at the default: true.
	// SaveArgs.Error = ???; // I think we can leave this at the default: GError.
	// SaveArgs.SavePAckageContext = ???; // I think we can leave this at the default: nullptr.
	bool bSaved = UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);
#endif
	if (!bSaved)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Unreal Engine unable to save package '%s' to file '%s'."),
			*PackagePath, *PackageFilename);
		return false;
	}

	return true;
}

FRawMesh FAGX_EditorUtilities::CreateRawMeshFromTrimesh(const FTrimeshShapeBarrier& Trimesh)
{
	if (Trimesh.GetNumPositions() <= 0 || Trimesh.GetNumIndices() <= 0)
	{
		// No collision mesh data available, this trimesh is invalid.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported trimesh '%s'. Cannot create "
				 "StaticMesh asset."),
			*Trimesh.GetSourceName())
		return FRawMesh();
	}

	if (Trimesh.GetNumIndices() != Trimesh.GetNumTriangles() * 3)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The trimesh '%s' does not have three vertex indices per triangle. The mesh "
				 "may be be imported incorrectly. Found %d triangles and %d indices."),
			*Trimesh.GetSourceName(), Trimesh.GetNumTriangles(), Trimesh.GetNumIndices());
	}

	// Data we have:
	//
	// The collision data consists of three arrays: positions, indices, and normals. The
	// positions is a list of vertex positions. The indices comes in triplets for each triangle
	// and the triplet defines which vertex positions form that triangle. There is one normal
	// per triangle and they are stored in the same order as the position index triplets.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3 Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |  Triangle 0   |  Triangle 1   | ... |
	//   indices:    | int, int, int | int, int, int | ... | Index into positions.
	//   normals:    |     Vec3      |      Vec3     | ... |
	//
	//
	// Data we want:
	//
	// Unreal Engine store its meshes in a similar format but with additional vertex data stored per
	// triangle. The Unreal Engine format consists of six arrays: positions, indices, tangent X,
	// tangent Y, tangent Z, and texture coordinates. Tangent Z has the same meaning as the normal
	// in the AGX Dynamics data. I don't know how to compute the X and Y tangents, but there are
	// flags to auto-compute them. See AddRawMeshToStaticMesh.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |    Triangle 0    |    Triangle 1    | ... |
	//    indices:   | int,  int,  int  | int,  int,  int  | ... | Index into positions.
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | 3x collision normal per triangle.
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... | (0, 0) everywhere.
	//    colors:    | Fcol, FCol, FCol | Fcol, FCol, FCol | ... | White everywhere.
	//    material:  |       int        |       int        | ... | 0 everywhere.
	//    smoothing: |      uint        |      uint        | ... | 0 everywhere, i.e., no smoothing.
	//
	// The positions and indices can simply be copied over. The normals must be triplicated over
	// all three vertices of each triangle. The other tangents are left unset and
	// bRecomputeTangents enabled in the StaticMesh SourceModel settings, see
	// AddRawMeshToStaticMesh. Texture coordinates are set to (0, 0) since the Trimesh doesn't have
	// any texture coordinates.

	FRawMesh RawMesh;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	RawMesh.VertexPositions = Trimesh.GetVertexPositions();
#else
	const TArray<FVector>& TrimeshVertexPositions = Trimesh.GetVertexPositions();
	RawMesh.VertexPositions.SetNum(TrimeshVertexPositions.Num());
	for (int32 I = 0; I < TrimeshVertexPositions.Num(); ++I)
	{
		// May do a double -> float conversion, depending on the UE_LARGE_WORLD_COORDINATES_DISABLED
		// preprocessor macro.
		RawMesh.VertexPositions[I] = ToMeshVector(TrimeshVertexPositions[I]);
	}
#endif
	RawMesh.WedgeIndices = Trimesh.GetVertexIndices();

	const int32 NumTriangles = Trimesh.GetNumTriangles();
	const int32 NumIndices = Trimesh.GetNumIndices();

	// Buffers with three elements per triangle, i.e., one per wedge.
	RawMesh.WedgeTangentZ.Reserve(NumIndices);
	RawMesh.WedgeColors.Reserve(NumIndices);
	RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

	// Buffers with one element per triangle.
	RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
	RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);

	const TArray<FVector> TriangleNormals = Trimesh.GetTriangleNormals();

	for (int32 TIdx = 0; TIdx < NumTriangles; ++TIdx)
	{
		// I don't know how to compute the first two tangents, but bRecomputeTangents (not
		// bRecomputeNormals) has been enabled on the StaticMesh SourceModel in
		// AddRawMeshToStaticMesh. Perhaps that's enough.

		// Since we replicate the same normal for all vertices of a triangle we will get a
		// flat-shaded mesh, should this mesh ever be used for rendering.
		const FVector3f Normal = ToMeshVector(TriangleNormals[TIdx]);
		RawMesh.WedgeTangentZ.Add(Normal);
		RawMesh.WedgeTangentZ.Add(Normal);
		RawMesh.WedgeTangentZ.Add(Normal);

		// The collision mesh doesn't have color information, so just write white.
		const FColor Color(255, 255, 255);
		RawMesh.WedgeColors.Add(Color);
		RawMesh.WedgeColors.Add(Color);
		RawMesh.WedgeColors.Add(Color);

		// We must write something to the texture coordinates or else Unreal Engine crashes when
		// processing this mesh later. We could try to do something clever here, but I think just
		// writing zero everywhere is safest.
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});

		// The collision mesh doesn't have material slots, so the best we can do is to provide a
		// single material and apply it to every triangle.
		RawMesh.FaceMaterialIndices.Add(0);

		// Not entirely sure on the FaceSmoothingMasks, the documentation is a little vague:
		//     Smoothing mask. Array[FaceId] = uint32
		// But I believe the process is that Unreal Engine does bitwise-and between two neighboring
		// faces and if the result comes out as non-zero then smoothing will happen along that
		// edge. Not sure what is being smoothed though. Perhaps the vertex normals are merged
		// if smoothing is on, and kept separate if smoothing is off. Also not sure how this
		// relates to the bRecompute.* settings on the StaticMesh's SourceModel.
		RawMesh.FaceSmoothingMasks.Add(0x00000000);
	}

	return RawMesh;
}

FRawMesh FAGX_EditorUtilities::CreateRawMeshFromRenderData(const FRenderDataBarrier& RenderData)
{
	if (RenderData.GetNumTriangles() <= 0)
	{
		// No render mesh data available, this render data is invalid.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported render data '%s'. Cannot create "
				 "StaticMesh asset."),
			*RenderData.GetGuid().ToString());
		return FRawMesh();
	}

	// What we have:
	//
	// The render data consists of four arrays: positions, normals, texture coordinates, and
	// indices. The function is similar to the collision data, but this time everything is
	// indexed and all arrays except for the index array create a single conceptual Vertex
	// struct.
	//
	// Data shared among triangles:
	//    positions:  [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    normals:    [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    tex coords: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
	//
	// Data owned by each triangle:
	//                |  Triangle 0   |  Triangle 1   | ... |
	//    indices:    | int, int, int | int, int, int | ... |
	//
	//
	// What we want:
	//
	// Unreal Engine store its meshes in a format similar to the render format in AGX Dynamics, but
	// more data is owned per triangle instead of shared between multiple triangles. The Unreal
	// Engine format consists of six arrays: positions, indices, tangent X, tangent Y, tangent Z,
	// and texture coordinates. Tangent Z has the same meaning as the normal in the AGX Dynamics
	// data.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |    Triangle 0    |    Triangle 1    | ... |
	//    indices:   | int,  int,  int  | int,  int,  int  | ... | Index into positions.
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Copied from Render Data.
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... | Copied from Render Data.
	//    colors:    | FCol, FCol, FCol | FCol, FCol, FCol | ... | While everywhere.
	//    material:  |       int        |       int        | ... | 0 everywhere.
	//    smoothing: |      uint        |      uint        | ... | All-1 everywhere.

	FRawMesh RawMesh;

	// A straight up copy of the vertex positions may be wasteful since the render data may
	// contain duplicated positions with different normals or texture coordinates. Sine Unreal
	// Engine decouples the normals and the texture coordinates from the positions we may end up
	// with useless position duplicates. If this becomes a serious concern, then find a way to
	// remove duplicates and patch the WedgeIndies to point to the correct merged vertex position.
	// Must use the render vertex indices in the per-index conversion loop below.
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	RawMesh.VertexPositions = RenderData.GetPositions();
#else
	const TArray<FVector>& TrimeshVertexPositions = RenderData.GetPositions();
	RawMesh.VertexPositions.SetNum(TrimeshVertexPositions.Num());
	for (int32 I = 0; I < TrimeshVertexPositions.Num(); ++I)
	{
		// May do a double -> float conversion, depending on the UE_LARGE_WORLD_COORDINATES_DISABLED
		// preprocessor macro.
		RawMesh.VertexPositions[I] = ToMeshVector(TrimeshVertexPositions[I]);
	}
#endif
	RawMesh.WedgeIndices = RenderData.GetIndices();

	const int32 NumTriangles = RenderData.GetNumTriangles();
	const int32 NumIndices = RenderData.GetNumIndices();

	RawMesh.WedgeTangentZ.Reserve(NumIndices);
	RawMesh.WedgeColors.Reserve(NumIndices);
	RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

	const TArray<FVector> RenderNormals = RenderData.GetNormals();
	const auto RenderTexCoords = RenderData.GetTextureCoordinates();

	for (int32 I = 0; I < NumIndices; ++I)
	{
		const int32 RenderI = RawMesh.WedgeIndices[I];
		RawMesh.WedgeTangentZ.Add(ToMeshVector(RenderNormals[RenderI]));
		// Not all Render Data has texture coordinates.
		if (RenderTexCoords.Num() > I)
		{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			RawMesh.WedgeTexCoords[0].Add(RenderTexCoords[RenderI]);
#else
			RawMesh.WedgeTexCoords[0].Add(
				FVector2f {(float) RenderTexCoords[RenderI].X, (float) RenderTexCoords[RenderI].Y});
#endif
		}
		else
		{
			RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		}
		RawMesh.WedgeColors.Add(FColor(255, 255, 255));
	}

	RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
	RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);
	for (int32 I = 0; I < NumTriangles; ++I)
	{
		RawMesh.FaceMaterialIndices.Add(0);
		RawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF);
	}

	return RawMesh;
}

void FAGX_EditorUtilities::AddRawMeshToStaticMesh(FRawMesh& RawMesh, UStaticMesh* StaticMesh)
{
#if UE_VERSION_OLDER_THAN(4, 27, 0)
	StaticMesh->StaticMaterials.Add(FStaticMaterial());
#else
	StaticMesh->GetStaticMaterials().Add(FStaticMaterial());
#endif

#if UE_VERSION_OLDER_THAN(4, 23, 0)
	StaticMesh->SourceModels.Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
#elif UE_VERSION_OLDER_THAN(5, 0, 0)
	StaticMesh->GetSourceModels().Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModels().Last();
#else
	FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
#endif

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	// There is a SaveRawMesh on the source model as well, but calling that causes a failed assert.
	// Is that a sign that we're doing something we shouldn't and the engine doesn't detect it
	// because we're sidestepping the safety checks? Or is it OK to do it this way?
	SourceModel.RawMeshBulkData->SaveRawMesh(RawMesh);
#else
	SourceModel.SaveRawMesh(RawMesh);
#endif

	FMeshBuildSettings& BuildSettings = SourceModel.BuildSettings;

	// Somewhat unclear what all these should be. Setting everything I don't understand to false.
	BuildSettings.bRecomputeNormals = false;
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bUseMikkTSpace = true;
	BuildSettings.bGenerateLightmapUVs = true;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	BuildSettings.bBuildAdjacencyBuffer = false;
#endif
	BuildSettings.bBuildReversedIndexBuffer = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;
}

UStaticMeshComponent* FAGX_EditorUtilities::CreateStaticMeshComponent(
	AActor& Owner, USceneComponent& Outer, UStaticMesh& MeshAsset, bool bRegisterComponent)
{
	/// \todo Which EObjectFlags should be passed to NewObject?
	UStaticMeshComponent* StaticMeshComponent =
		NewObject<UStaticMeshComponent>(&Outer, FName(*MeshAsset.GetName()));
	StaticMeshComponent->SetStaticMesh(&MeshAsset);
	Owner.AddInstanceComponent(StaticMeshComponent);
	if (bRegisterComponent)
	{
		StaticMeshComponent->RegisterComponent();
	}
	if (!StaticMeshComponent->AttachToComponent(
			&Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to attach imported StaticMeshComponent '%s' to parent Component '%s' in "
				 "Actor '%s'."),
			*StaticMeshComponent->GetName(), *Outer.GetName(), *Owner.GetName());
	}
	return StaticMeshComponent;
}

FString FAGX_EditorUtilities::CreateShapeMaterialAsset(
	const FString& DirName, const FShapeMaterialBarrier& Material)
{
	FString MaterialName =
		CreateAssetName(Material.GetName(), DirName, TEXT("ImportedAGXMaterial"));

	// Find actual package path and a unique asset name.
	FString PackagePath = FString::Printf(TEXT("/Game/ImportedAGXShapeMaterials/%s/"), *DirName);
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackagePath, MaterialName, PackagePath, MaterialName);

	// Create the package that will hold our shape material asset.
#if UE_VERSION_OLDER_THAN(4, 26, 0)
	UPackage* Package = CreatePackage(nullptr, *PackagePath);
#else
	UPackage* Package = CreatePackage(*PackagePath);
#endif

#if 0
		/// \todo Unclear if this is needed or not. Leaving it out for now but
		/// test with it restored if there are problems.
		Package->FullyLoad();
#endif

	UAGX_ShapeMaterialAsset* MaterialAsset = NewObject<UAGX_ShapeMaterialAsset>(
		Package, FName(*MaterialName), RF_Public | RF_Standalone);

	// Copy material properties to the new material asset.
	MaterialAsset->CopyFrom(&Material);

	bool Saved = FinalizeAndSavePackage(Package, MaterialAsset, PackagePath, MaterialName);
	if (!Saved)
	{
		// Return empty string if asset was not created properly.
		return FString();
	}

	return PackagePath;
}

AAGX_ConstraintActor* FAGX_EditorUtilities::CreateConstraintActor(
	UClass* ConstraintType, UAGX_RigidBodyComponent* RigidBody1,
	UAGX_RigidBodyComponent* RigidBody2, bool bInPlayingWorldIfAvailable, bool bSelect,
	bool bShowNotification)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();

	check(World);
	check(ConstraintType->IsChildOf<AAGX_ConstraintActor>());

	// Create the new Constraint Actor.
	AAGX_ConstraintActor* NewActor =
		World->SpawnActorDeferred<AAGX_ConstraintActor>(ConstraintType, FTransform::Identity);

	check(NewActor);

	/// \todo We have the Component we want. There should be a way to specify it directly, without
	/// being dependent on its name.
	UAGX_ConstraintComponent* Constraint = NewActor->GetConstraintComponent();
	Constraint->BodyAttachment1.RigidBody.OwningActor = RigidBody1->GetOwner();
	Constraint->BodyAttachment1.RigidBody.BodyName = RigidBody1->GetFName();
	if (RigidBody2 != nullptr)
	{
		Constraint->BodyAttachment2.RigidBody.OwningActor = RigidBody2->GetOwner();
		Constraint->BodyAttachment2.RigidBody.BodyName = RigidBody2->GetFName();
	}
	else
	{
		Constraint->BodyAttachment2.RigidBody.OwningActor = nullptr;
		Constraint->BodyAttachment2.RigidBody.BodyName = NAME_None;
	}

	NewActor->FinishSpawning(FTransform::Identity, true);

	if (bSelect)
	{
		SelectActor(NewActor);
	}

	if (bShowNotification)
	{
		ShowNotification(LOCTEXT("CreateConstraintSucceeded", "AGX Constraint Created"));
	}

	return NewActor;
}

UAGX_ConstraintComponent* FAGX_EditorUtilities::CreateConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2,
	UClass* ConstraintType)
{
	UAGX_ConstraintComponent* Constraint =
		NewObject<UAGX_ConstraintComponent>(Owner, ConstraintType);
	Owner->AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	Constraint->BodyAttachment1.RigidBody.OwningActor = RigidBody1->GetOwner();
	Constraint->BodyAttachment1.RigidBody.BodyName = RigidBody1->GetFName();
	if (RigidBody2 != nullptr)
	{
		Constraint->BodyAttachment2.RigidBody.OwningActor = RigidBody2->GetOwner();
		Constraint->BodyAttachment2.RigidBody.BodyName = RigidBody2->GetFName();
	}
	else
	{
		Constraint->BodyAttachment2.RigidBody.OwningActor = nullptr;
		Constraint->BodyAttachment2.RigidBody.BodyName = NAME_None;
	}
	return Constraint;
}

UAGX_HingeConstraintComponent* FAGX_EditorUtilities::CreateHingeConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2)
{
	return CreateConstraintComponent<UAGX_HingeConstraintComponent>(Owner, Body1, Body2);
}

UAGX_PrismaticConstraintComponent* FAGX_EditorUtilities::CreatePrismaticConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2)
{
	return CreateConstraintComponent<UAGX_PrismaticConstraintComponent>(Owner, Body1, Body2);
}

AAGX_ConstraintFrameActor* FAGX_EditorUtilities::CreateConstraintFrameActor(
	AActor* ParentActor, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();
	check(World);

	// Create the new Constraint Frame Actor.
	AAGX_ConstraintFrameActor* NewActor = World->SpawnActor<AAGX_ConstraintFrameActor>();
	check(NewActor);

	// Set the new actor as child to the Rigid Body.
	if (ParentActor)
	{
		if (ParentActor->GetWorld() == World)
		{
			NewActor->AttachToActor(ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			UE_LOG(
				LogAGX, Log,
				TEXT("Failed to attach the new AGX Constraint Frame Actor to the specified "
					 "Parent Rigid Body Actor, because it is in another World."));
		}
	}

	if (bSelect)
	{
		SelectActor(NewActor);
	}

	if (bShowNotification)
	{
		ShowNotification(
			LOCTEXT("CreateConstraintFrameActorSucceded", "AGX Constraint Frame Actor Created"));
	}

	return NewActor;
}

void FAGX_EditorUtilities::SelectActor(AActor* Actor, bool bDeselectPrevious)
{
	if (bDeselectPrevious)
	{
		GEditor->SelectNone(
			/*bNoteSelectionChange*/ false,
			/*bDeselectBSPSurfs*/ true,
			/*WarnAboutManyActors*/ false);
	}

	if (Actor)
	{
		GEditor->SelectActor(
			Actor,
			/*bInSelected*/ true,
			/*bNotify*/ false);
	}

	GEditor->NoteSelectionChange();
}

void FAGX_EditorUtilities::ShowNotification(const FText& Text)
{
	FNotificationInfo Info(Text);
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 5.0f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();
	// GEditor->PlayEditorSound(CompileSuccessSound);
}

UWorld* FAGX_EditorUtilities::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

UWorld* FAGX_EditorUtilities::GetPlayingWorld()
{
	// Without starting from an Actor, the world needs to be found
	// in another way:

	TArray<APlayerController*> PlayerControllers;
	GEngine->GetAllLocalPlayerControllers(PlayerControllers);

	if (PlayerControllers.Num() > 0)
	{
		return PlayerControllers[0]->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

UWorld* FAGX_EditorUtilities::GetCurrentWorld()
{
	if (UWorld* PlayingWorld = GetPlayingWorld())
	{
		return PlayingWorld;
	}
	else
	{
		return GetEditorWorld();
	}
}

void FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(
	AActor** OutActor1, AActor** OutActor2, bool bSearchSubtrees, bool bSearchAncestors)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();

	if (!SelectedActors)
		return;

	if (OutActor1)
		*OutActor1 = nullptr;

	if (OutActor2)
		*OutActor2 = nullptr;

	// Assigns to first available of OutActor1 and OutActor2, and returns whether
	// at least one of them is afterwards still available for assignment.
	auto AssignOutActors = [OutActor1, OutActor2](AActor* RigidBodyActor)
	{
		if (OutActor1 && *OutActor1 == nullptr)
		{
			*OutActor1 = RigidBodyActor;
		}
		// Making sure same actor is not used for both OutActors.
		else if (OutActor2 && *OutActor2 == nullptr && (!OutActor1 || *OutActor1 != RigidBodyActor))
		{
			*OutActor2 = RigidBodyActor;
		}

		return (OutActor1 && *OutActor1 == nullptr) || (OutActor2 && *OutActor2 == nullptr);
	};

	// Search the selected actors fpr matching actors. Doing this step completely before
	// start searching in subtrees, in case selected actors are in each others subtrees.
	for (int32 i = 0; i < SelectedActors->Num(); ++i)
	{
		if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
		{
			if (UAGX_RigidBodyComponent::GetFirstFromActor(SelectedActor))
			{
				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(SelectedActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}

	// Search each selected actor's subtree for matching actors. Only one matching actor
	// allowed per selected actor subtree.
	if (bSearchSubtrees)
	{
		for (int32 i = 0; i < SelectedActors->Num(); ++i)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
			{
				AActor* RigidBodyActor =
					GetRigidBodyActorFromSubtree(SelectedActor, (OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}

	// Search each selected actor's ancestor chain for matching actors. Only one matching actor
	// allowed per selected actor ancestor chain.
	if (bSearchAncestors)
	{
		for (int32 i = 0; i < SelectedActors->Num(); ++i)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
			{
				AActor* RigidBodyActor = GetRigidBodyActorFromAncestors(
					SelectedActor, (OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromSubtree(
	AActor* SubtreeRoot, const AActor* IgnoreActor)
{
	AActor* RigidBodyActor = nullptr;

	if (SubtreeRoot)
	{
		if (SubtreeRoot != IgnoreActor && UAGX_RigidBodyComponent::GetFirstFromActor(SubtreeRoot))
		{
			RigidBodyActor = SubtreeRoot; // found it
		}
		else
		{
			TArray<AActor*> AttachedActors;
			SubtreeRoot->GetAttachedActors(AttachedActors);

			for (AActor* AttachedActor : AttachedActors)
			{
				RigidBodyActor = GetRigidBodyActorFromSubtree(AttachedActor, IgnoreActor);

				if (RigidBodyActor)
				{
					break; // found it
				}
			}
		}
	}

	return RigidBodyActor;
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromAncestors(
	AActor* Actor, const AActor* IgnoreActor)
{
	AActor* RigidBodyActor = nullptr;

	if (Actor)
	{
		if (Actor != IgnoreActor && UAGX_RigidBodyComponent::GetFirstFromActor(Actor))
		{
			RigidBodyActor = Actor;
		}
		else
		{
			RigidBodyActor =
				GetRigidBodyActorFromAncestors(Actor->GetAttachParentActor(), IgnoreActor);
		}
	}

	return RigidBodyActor;
}

void FAGX_EditorUtilities::GetAllClassesOfType(
	TArray<UClass*>& OutMatches, UClass* BaseClass, bool bIncludeAbstract)
{
	for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
	{
		UClass* Class = *ClassItr;

		if (Class && Class->IsChildOf(BaseClass))
		{
			if (bIncludeAbstract || !Class->HasAnyClassFlags(CLASS_Abstract))
			{
				OutMatches.Add(Class);
			}
		}
	}
}

bool FAGX_EditorUtilities::ApplyShapeMaterial(
	UAGX_ShapeComponent* Shape, const FString& ShapeMaterialAsset)
{
	if (!Shape)
	{
		return false;
	}

	// Get the ShapeMaterialAsset.
	UAGX_ShapeMaterialAsset* MaterialAsset =
		GetAssetByPath<UAGX_ShapeMaterialAsset>(ShapeMaterialAsset);

	if (!MaterialAsset)
	{
		// Logging handled by GetAssetByPath().
		return false;
	}

	Shape->ShapeMaterial = MaterialAsset;
	return true;
}

EVisibility FAGX_EditorUtilities::VisibleIf(bool bVisible)
{
	return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FString FAGX_EditorUtilities::SelectExistingFileDialog(
	const FString& FileDescription, const FString& FileExtension)
{
	const FString DialogTitle = FString("Select an ") + FileDescription;
	const FString FileTypes = FileDescription + FString("|*") + FileExtension;
	// For a discussion on window handles see
	// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr, DialogTitle, TEXT("DefaultPath"), TEXT("DefaultFile"), FileTypes,
		EFileDialogFlags::None, Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Log, TEXT("No %s file selected. Doing nothing."), *FileExtension);
		return "";
	}
	if (Filenames.Num() > 1)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("Multiple files selected but only single file selection supported. Doing "
				 "nothing."));
		FAGX_EditorUtilities::ShowNotification(LOCTEXT(
			"Multiple files",
			"Multiple files selected but but only single file selection supported. Doing "
			"nothing."));
		return "";
	}
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Filenames[0]);
}

FString FAGX_EditorUtilities::SelectExistingDirectoryDialog(
	const FString& DialogTitle, const FString& InStartDir, bool AllowNoneSelected)
{
	const FString StartDir = InStartDir.IsEmpty() ? FString("DefaultPath") : InStartDir;
	FString DirectoryPath("");
	bool DirectorySelected = FDesktopPlatformModule::Get()->OpenDirectoryDialog(
		nullptr, DialogTitle, StartDir, DirectoryPath);

	if (!AllowNoneSelected && (!DirectorySelected || DirectoryPath.IsEmpty()))
	{
		UE_LOG(LogAGX, Log, TEXT("No directory selected. Doing nothing."));
		return "";
	}

	return DirectoryPath;
}

FString FAGX_EditorUtilities::SelectNewFileDialog(
	const FString& DialogTitle, const FString& FileExtension, const FString& FileTypes,
	const FString& DefaultFile, const FString& InStartDir)
{
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->SaveFileDialog(
		nullptr, DialogTitle, InStartDir, DefaultFile, FileTypes, EFileDialogFlags::None,
		Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("No file selected, doing nothing."));
		return "";
	}

	if (Filenames.Num() > 1)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Multiple files selected but we only support selecting one. Doing nothing."));
		return "";
	}

	const FString Filename = Filenames[0];
	if (Filename.IsEmpty())
	{
		UE_LOG(LogAGX, Warning, TEXT("Selected file has empty file name. Doing nothing."));
		return "";
	}

	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Filename);
}

#undef LOCTEXT_NAMESPACE
