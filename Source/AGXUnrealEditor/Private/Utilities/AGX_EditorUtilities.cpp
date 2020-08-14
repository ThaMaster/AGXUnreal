#include "Utilities/AGX_EditorUtilities.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
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
#include "Misc/MessageDialog.h"
#include "Misc/EngineVersionComparison.h"
#include "RawMesh.h"
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
		const bool Attached = Shape->AttachToComponent(
			Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		check(Attached);
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

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(
	AActor* Owner, USceneComponent* Outer, bool bRegister)
{
	return ::CreateShapeComponent<UAGX_TrimeshShapeComponent>(Owner, Outer, bRegister);
}

namespace
{
	FRawMesh CreateRawMeshFromCollisionData(const FTrimeshShapeBarrier& Trimesh)
	{
		// What we have:
		//
		// Data shared among triangles:
		//   positions: [Vec3, Vec3, Vec3 Vec3, Vec3, ... ]
		//
		// Data owned by each triangle:
		//   indices:    | int, int, int | int, int, int | ... |
		//   normal:     |     Vec3      |      Vec3     | ... |
		//               |  Triangle 0   |  Triangle 1   | ... |
		//
		//
		// What we want:
		//
		// Data shared among triangles:
		//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//
		// Data owned by each triangle:
		//    indices:   | int,  int,  int  | int,  int,  int  | ... |
		//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... |
		//
		// The positions and indices can simply be copied over. The normals must be triplicated over
		// all three vertices of each triangle. The other tangents are left unset and
		// bRecomputeTangents enabled in the StaticMesh SourceModel settings. Texture coordinates
		// are left empty.

		if (Trimesh.GetNumIndices() != Trimesh.GetNumTriangles() * 3)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("The trimesh '%s' does not have three vertex indices per triangle. The mesh "
					 "may be be imported incorrectly. Found %d triangles and %d indices."),
				*Trimesh.GetSourceName(), Trimesh.GetNumTriangles(), Trimesh.GetNumIndices());
		}

		FRawMesh RawMesh;

		RawMesh.VertexPositions = Trimesh.GetVertexPositions();
		RawMesh.WedgeIndices = Trimesh.GetVertexIndices();

		const int32 NumTriangles = Trimesh.GetNumTriangles();
		const int32 NumIndices = Trimesh.GetNumIndices();

		RawMesh.WedgeTangentZ.Reserve(NumIndices);
		RawMesh.WedgeColors.Reserve(NumIndices);
		RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

		// Unreal Engine data that has no correspondent in the AGX Dynamics data.
		RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
		RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);

		const TArray<FVector> TriangleNormals = Trimesh.GetTriangleNormals();

		for (int32 TIdx = 0; TIdx < NumTriangles; ++TIdx)
		{
			// I don't know how to compute the first two two tangents, but bRecomputeTangents (not
			// bRecomputeNormals) has been enabled on the StaticMesh SourceModel. Perhaps that's
			// enough.

			FVector Normal = TriangleNormals[TIdx];
			RawMesh.WedgeTangentZ.Add(Normal);
			RawMesh.WedgeTangentZ.Add(Normal);
			RawMesh.WedgeTangentZ.Add(Normal);

			RawMesh.WedgeColors.Add(FColor(255, 255, 255));
			RawMesh.WedgeColors.Add(FColor(255, 255, 255));
			RawMesh.WedgeColors.Add(FColor(255, 255, 255));

			/// \todo We should do something cleverer here. Perhaps project position onto a primary
			/// plane or, if incompatible render data is available, find the nearest render vertex
			/// and use that texture coordinate.
			FVector2D TexCoord(0.0f, 0.0f);
			RawMesh.WedgeTexCoords[0].Add(TexCoord);
			RawMesh.WedgeTexCoords[0].Add(TexCoord);
			RawMesh.WedgeTexCoords[0].Add(TexCoord);

			RawMesh.FaceMaterialIndices.Add(0);
			RawMesh.FaceSmoothingMasks.Add(0x00000000);
			// Not entirely sure on the FaceSmoothingMasks, the documentation is a little wague:
			//     Smoothing mask. Array[FaceId] = uint32
			// But I believe the process is that Unreal Engine does bit-and between two neighboring
			// faces and if the result comes out as non-zero then smoothing will happen along that
			// edge. Not sure what is being smoothed though. Perhaps the vertex normals are merged
			// if smoothing is on, and kept separate if smoothing is off. Also not sure how this
			// relates to the bRecompute.* settings on the StaticMesh's SourceModel.
		}

		return RawMesh;
	}

	FRawMesh CreateRawMeshFromRenderData(const FTrimeshShapeBarrier& Trimesh)
	{
		// What we have:
		//
		// Data shared among triangles:
		//    positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//    normals:   [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//    tex coord: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
		//
		// Data owned by each triangle:
		//    indices:   | int, int, int | int, int, int | ... |
		//               |  Triangle 0   |  Triangle 1   | ... |
		//
		//
		// What we want:
		//
		// Data shared among triangles:
		//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//
		// Data owned by each triangle:
		//    indices:   | int,  int,  int  | int,  int,  int  | ... |
		//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... |

		FRawMesh RawMesh;

		// A straight up copy of the vertex positions may be wasteful since the render data may
		// contain duplicated positions with different normals or texture coordinates. If this
		// becomes a serious concern, then find a way to remove duplicates and patch the WedgeIndies
		// to point to the correct merged vertex position. Must use the render vertex indices in the
		// per-index conversion loop below.
		RawMesh.VertexPositions = Trimesh.GetRenderDataPositions();
		RawMesh.WedgeIndices = Trimesh.GetRenderDataIndices();

		const int32 NumTriangles = Trimesh.GetNumRenderTriangles();
		const int32 NumIndices = Trimesh.GetNumRenderIndices();

		RawMesh.WedgeTangentZ.Reserve(NumIndices);
		RawMesh.WedgeColors.Reserve(NumIndices);
		RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

		const TArray<FVector> RenderNormals = Trimesh.GetRenderDataNormals();
		const TArray<FVector2D> RenderCoordinates = Trimesh.GetRenderDataTextureCoordinates();

		for (int32 I = 0; I < NumIndices; ++I)
		{
			const int32 RenderI = RawMesh.WedgeIndices[I];
			RawMesh.WedgeTangentZ.Add(RenderNormals[RenderI]);
			RawMesh.WedgeTexCoords[0].Add(RenderCoordinates[RenderI]);
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

	FRawMesh CreateRawMeshFromCollisionAndRenderData(const FTrimeshShapeBarrier& Trimesh)
	{
		// What we have, collision:
		//
		// Data shared among triangles:
		//   positions: [Vec3, Vec3, Vec3 Vec3, Vec3, ... ]
		//
		// Data owned by each triangle:
		//   indices:    | int, int, int | int, int, int | ... |
		//   normal:     |     Vec3      |      Vec3     | ... |
		//               |  Triangle 0   |  Triangle 1   | ... |
		//
		// What we have, render:
		//
		// Data shared among triangles:
		//    positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//    normals:   [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//    tex coord: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
		//
		// Data owned by each triangle:
		//    indices:   | int, int, int | int, int, int | ... |
		//               |  Triangle 0   |  Triangle 1   | ... |
		//
		//
		// What we want:
		//
		// Data shared among triangles:
		//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
		//
		// Data owned by each triangle:
		//    indices:   | int,  int,  int  | int,  int,  int  | ... |
		//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
		//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... |
		//
		// The positions and indices can simply be copied over. The normals and texture coordinates
		// must be applied per triangle vertex instead of stored with the shared vertex data.

		check(Trimesh.GetNumRenderIndices() == Trimesh.GetNumIndices());
		check(Trimesh.GetNumRenderTriangles() == Trimesh.GetNumTriangles());
		if (Trimesh.GetNumIndices() != Trimesh.GetNumTriangles() * 3)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("The trimesh '%s' does not have three vertex indices per triangle. The mesh "
					 "may be be imported incorrectly. Found %d triangles and %d indices."),
				*Trimesh.GetSourceName(), Trimesh.GetNumTriangles(), Trimesh.GetNumIndices());
		}

		FRawMesh RawMesh;

		// Here we assume that the collision mesh and the render mesh are equivalent, i.e., that
		// they describe the same triangles in the same order. That is, we assume that
		//   Collision.Position[Collision.Index[I]] == Render.Position[Collision.Index[I]]
		// for all I in [0 ... 3*#tris).
		// This assumption allows us to use the collision mesh positions, which is often much fewer,
		// and still apply per-wedge normals and texture coordinates from the render mesh.
		RawMesh.VertexPositions = Trimesh.GetVertexPositions();
		RawMesh.WedgeIndices = Trimesh.GetVertexIndices();
		const TArray<uint32> RenderDataIndices = Trimesh.GetRenderDataIndices();

		const int32 NumTriangles = Trimesh.GetNumRenderTriangles();
		const int32 NumIndices = Trimesh.GetNumRenderIndices();

		// Not touching WedgeTangent[XY] because I don't know how to compute them and
		// bRecomputeTangents has been set to true on the StaticMesh's SourceModel.
		RawMesh.WedgeTangentZ.Reserve(NumIndices);
		RawMesh.WedgeColors.Reserve(NumIndices);
		RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

		// Lookup into these should use indices from the RenderDataIndices array.
		const TArray<FVector> RenderNormals = Trimesh.GetRenderDataNormals();
		const TArray<FVector2D> RenderCoordinates = Trimesh.GetRenderDataTextureCoordinates();

		// Gather data that is per-vertex indexed in the AGX Dynamics format but per-wedge in the
		// Unreal Engine format.
		for (int32 I = 0; I < NumIndices; ++I)
		{
			const int32 RenderI = RenderDataIndices[I];
			RawMesh.WedgeTangentZ.Add(RenderNormals[RenderI]);
			RawMesh.WedgeTexCoords[0].Add(RenderCoordinates[RenderI]);
			RawMesh.WedgeColors.Add(FColor(255, 255, 255));
		}

		// A single face material index is used for all triangles.
		// We have nice normals, so enable smooth shading for all triangles.
		RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
		RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);
		for (int32 I = 0; I < NumTriangles; ++I)
		{
			RawMesh.FaceMaterialIndices.Add(0);
			RawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF);
		}

		return RawMesh;
	}
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

	bool bSaved = UPackage::SavePackage(Package, Asset, RF_NoFlags, *PackageFilename);
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
	// AGX Dynamics store mesh data in two formats: collision and render. The render portion is
	// optional.
	//
	//
	// Collision
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
	//   indices:    | int, int, int | int, int, int | ... |
	//   normals:    |     Vec3      |      Vec3     | ... |
	//               |  Triangle 0   |  Triangle 1   | ... |
	//
	//
	// Render
	//
	// The render data consists of four arrays: positions, normals, texture coordinates, and
	// indices. The function is similar to the collision data, but this time everything is
	// indexed and all arrays except for the index array create a single conceptual Vertex
	// struct.
	//
	// Data shared among triangles:
	//    positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    normals:   [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    tex coord: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
	//
	// Data owned by each triangle:
	//    indices:   | int, int, int | int, int, int | ... |
	//               |  Triangle 0   |  Triangle 1   | ... |
	//
	//
	// Unreal Engine store its meshes in a third format. It is similar to the render format in
	// AGX Dynamics, but more data is owned per triangle instead of shared between multiple
	// triangles. Another way of saying the same thing is that it is similar to the collision
	// format in AGX Dynamics, but with additional data owned by each triangle. The Unreal
	// Engine format consists of six arrays: positions, indices, tangent X, tangent Y, tangent
	// Z, and texture coordinates. Tangent Z has the same meaning as the normal in the AGX
	// Dynamics data.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//    indices:   | int,  int,  int  | int,  int,  int  | ... |
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... |
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... |
	//
	//
	// The AGX Dynamics to Unreal Engine mesh conversion uses vertex positions from the
	// collision data, and normals and texture coordinates from the render data if available and
	// compatible. By compatible we mean that the two meshes have the same number of triangles,
	// in which case we assume that the two meshes are equivalent and their data can be mixed.
	// There is no guarantee that this is true in all cases. If nor render data is available, or
	// if the meshes aren't compatible then the collision normals are used for rendering as
	// well. The same normal are used for all vertices within a triangle so the result will be
	// a flat-shaded triangle. No texture coordinates are written in this case. Ideas with
	// vertex position projections onto primary axis planes or nearest render vertex mapping has
	// been discussed but not implemented.

	const int32 NumCollisionPositions = Trimesh.GetNumPositions();
	const int32 NumRenderPositions = Trimesh.HasRenderData() ? Trimesh.GetNumRenderPositions() : 0;
	const int32 NumCollisionIndices = Trimesh.GetNumIndices();
	const int32 NumRenderIndices = Trimesh.HasRenderData() ? Trimesh.GetNumRenderIndices() : 0;

	if (NumCollisionPositions <= 0 || NumCollisionIndices <= 0)
	{
		// No collision mesh data available, this trimesh is broken.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported trimesh '%s'. Cannot create "
				 "StaticMesh asset."),
			*Trimesh.GetSourceName())
		return FRawMesh();
	}

	if (NumCollisionIndices == NumRenderIndices)
	{
		return CreateRawMeshFromCollisionAndRenderData(Trimesh);
	}
	else
	{
		return CreateRawMeshFromCollisionData(Trimesh);
	}
	// We could have a call to CreateRawMeshFromRenderData somewhere around here, if we had a
	// way to let the user request that.
}

void FAGX_EditorUtilities::AddRawMeshToStaticMesh(FRawMesh& RawMesh, UStaticMesh* StaticMesh)
{
	StaticMesh->StaticMaterials.Add(FStaticMaterial());
#if UE_VERSION_OLDER_THAN(4, 23, 0)
	StaticMesh->SourceModels.Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
#else
	StaticMesh->GetSourceModels().Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModels().Last();
#endif
	SourceModel.RawMeshBulkData->SaveRawMesh(RawMesh);
	FMeshBuildSettings& BuildSettings = SourceModel.BuildSettings;

	// Somewhat unclear what all these should be.
	BuildSettings.bRecomputeNormals = false;
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bUseMikkTSpace = true;
	BuildSettings.bGenerateLightmapUVs = true;
	BuildSettings.bBuildAdjacencyBuffer = false;
	BuildSettings.bBuildReversedIndexBuffer = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;
}

UStaticMeshComponent* FAGX_EditorUtilities::CreateStaticMeshComponent(
	AActor* Owner, UAGX_TrimeshShapeComponent* Outer, UStaticMesh* MeshAsset, bool bRegisterComponent)
{
	if (!MeshAsset)
	{
		UE_LOG(LogAGX, Error, TEXT("CreateStaticMeshComponent: parameter MeshAsset was nullptr."));
		return nullptr;
	}

	/// \todo Which EObjectFlags should be passed to NewObject?
	UStaticMeshComponent* StaticMeshComponent =
		NewObject<UStaticMeshComponent>(Outer, FName(*MeshAsset->GetName()));
	StaticMeshComponent->SetStaticMesh(MeshAsset);
	Owner->AddInstanceComponent(StaticMeshComponent);
	if (bRegisterComponent)
	{
		StaticMeshComponent->RegisterComponent();
	}
	const bool Attached = StaticMeshComponent->AttachToComponent(
		Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (!Attached)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to attach imported StaticMeshComponent '%s' to Actor '%s'."),
			*StaticMeshComponent->GetName(), *Owner->GetName());
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
	UPackage* Package = CreatePackage(nullptr, *PackagePath);
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

void FAGX_EditorUtilities::ShowDialogBox(const FText& Text)
{
	// Example of how an FText can be created:
	// FText DialogText = FText::Format(
	// 	LOCTEXT("PluginButtonDialogText", "{0} was recompiled at {1}.\n{2}"),
	// 	FText::FromString(TEXT(__FILE__)), FText::FromString(TEXT(__TIME__)),
	// 	FText::FromString(TEXT("Create body before root component.")));

	FMessageDialog::Open(EAppMsgType::Ok, Text);
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
	auto AssignOutActors = [OutActor1, OutActor2](AActor* RigidBodyActor) {
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

	Shape->PhysicalMaterial = MaterialAsset;
	return true;
}

#undef LOCTEXT_NAMESPACE
