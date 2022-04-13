// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "DetailLayoutBuilder.h"
#include "RawMesh.h"

// Standard library includes.
#include <tuple>

// Shape classes.
class FRenderDataBarrier;
class FShapeMaterialBarrier;
class FTrimeshShapeBarrier;
class UAGX_BoxShapeComponent;
class UAGX_CapsuleShapeComponent;
class UAGX_CylinderShapeComponent;
class UAGX_ShapeComponent;
class UAGX_SphereShapeComponent;
class UAGX_TrimeshShapeComponent;

// Constraint classes.
class AAGX_ConstraintActor;
class AAGX_ConstraintFrameActor;
class UAGX_ConstraintComponent;
class UAGX_HingeConstraintComponent;
class UAGX_PrismaticConstraintComponent;

// Other AGXUnreal classes.
class FContactMaterialBarrier;
class UAGX_RigidBodyComponent;

// Unreal Engine classes.
class AActor;
class FText;
class IDetailLayoutBuilder;
class UClass;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UWorld;

/**
 * A collection of helper functions that can only be compiled in editor builds.
 */
class AGXUNREALEDITOR_API FAGX_EditorUtilities
{
public:
	/**
	 * Create a new Actor with an empty USceneComponent as its RootComponent.
	 */
	static std::tuple<AActor*, USceneComponent*> CreateEmptyActor(
		const FTransform& Transform, UWorld* World);

	/**
	 * Create a new AGX Rigid Body Component as a child of the given Actor.
	 */
	static UAGX_RigidBodyComponent* CreateRigidBody(AActor* Owner);

	/**
	 * Create a new AGX Sphere Shape as child of the given Actor.
	 * The Shape will be attached to the given USceneComponent.
	 */
	static UAGX_SphereShapeComponent* CreateSphereShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new AGX Box Shape as child of the given Actor.
	 * The Shape will be attached to the given USceneComponent.
	 */
	static UAGX_BoxShapeComponent* CreateBoxShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new AGX Cylinder Shape as a child of the given Actor.
	 * The Shape will be attached to the given USceneComponent.
	 * @param Owner - The Actor that should own the new Cylinder Shape.
	 * @param Outer - The SceneComponent that the new Cylinder Shape should be attached to.
	 * @return A newly created Cylinder Shape.
	 */
	static UAGX_CylinderShapeComponent* CreateCylinderShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new AGX Capsule shape as a child of the given Actor.
	 * The Shape will be attached to the given USceneComponent.
	 * @param Owner - The Actor that should own the new Capsule Shape.
	 * @param Outer - The SceneComponent that the new Capsule Shape should be attached to.
	 * @return A newly created Capsule Shape.
	 */
	static UAGX_CapsuleShapeComponent* CreateCapsuleShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new AGX Trimesh Shape as a child of the given actor.
	 * The Shape will be attached to the given USceneComponent.
	 * A StaticMeshComponent is neither selected nor created.
	 * @see CreateStaticMesh.
	 * @param Owner The Actor that should own the newly created Component.
	 * @param Outer The USceneComponent that the newly created Component should be a attached to.
	 * @param bRegister True if AActor::registerComponent should be called. Must be called later if
	 * false.
	 */
	static UAGX_TrimeshShapeComponent* CreateTrimeshShape(
		AActor* Owner, USceneComponent* Outer, bool bRegister);

	/**
	 * Create an FRawMesh from the collision triangles in the given Trimesh Shape Barrier.
	 *
	 * The mesh created will be limited to the information stored in the Trimesh, so no texture
	 * coordinates and only one normal per triangle.
	 *
	 * @param Trimesh The Trimesh holding the triangles to convert.
	 * @return An FRawMesh containing the same triangles as the Trimesh.
	 */
	static FRawMesh CreateRawMeshFromTrimesh(const FTrimeshShapeBarrier& Trimesh);

	/**
	 * Create an FRawMesh from the render triangles in the given Render Data Barrier.
	 *
	 * Will return an empty FRawMesh if the Render Data doesn't have a mesh.
	 *
	 * @param RenderData Render Data holding the triangles to convert.
	 * @return An FRawMesh containing the same triangles as the Render Data.
	 */
	static FRawMesh CreateRawMeshFromRenderData(const FRenderDataBarrier& RenderData);

	/**
	 * Apply the RawMesh data to the StaticMesh.
	 *
	 * @param RawMesh - The RawMesh holding the mesh data.
	 * @param StaticMesh - The StaticMesh that should receive the mesh data.
	 */
	static void AddRawMeshToStaticMesh(FRawMesh& RawMesh, UStaticMesh* StaticMesh);

	/**
	 * Remove characters that are unsafe to use in object names, content
	 * references or file names. Unsupported characters are dropped, so check the
	 * returned string for emptyness.
	 *
	 * May remove more characters than necessary, and the set of allowed characters may be extended
	 * in the future.
	 *
	 * @param Name The name to sanitize.
	 * @return The name with all dangerous characters removed.
	 */
	static FString SanitizeName(const FString& Name);

	/**
	 * Remove characters that are unsafe to use in object names, content references or file names.
	 * Unsupported characters are dropped. If no characters remain then the fallback is returned.
	 *
	 * May remove more characters than necessary.
	 *
	 * @param Name The name to sanitize.
	 * @param Fallback Returned if none of the original characters remain.
	 * @return The name with all dangerous characters removed, or the fallback if all characters are
	 * dangerous.
	 */
	static FString SanitizeName(const FString& Name, const FString& Fallback);

	/**
	 * Remove characters that are unsafe to use in object names, content references or file names.
	 * Unsupported characters are dropped. If no characters remain then the fallback is returned.
	 *
	 * May remove more characters than necessary.
	 *
	 * @param Name The name to sanitize.
	 * @param Fallback Returned if none of the original characters remain.
	 * @return The name with all dangerous characters removed, or the fallback if all characters are
	 * dangerous.
	 */
	static FString SanitizeName(const FString& Name, const TCHAR* Fallback);

	static FString CreateAssetName(FString SourceName, FString ActorName, FString DefaultName);

	static void MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName);

	/**
	 * Save the given package/asset pair to disk.
	 * @param Package The package in which the asset will be saved..
	 * @param Asset The asset to save.
	 * @param PackagePath Asset path to the package. Often starting with "/Game/"
	 * @param AssetName The ane of the asset to save.
	 * @return The filename of the saved file, or the empty string on error.
	 */
	static bool FinalizeAndSavePackage(
		UPackage* Package, UObject* Asset, const FString& PackagePath, const FString& AssetName);

	/**
	 * Create a new UStaticMesh asset from the given mesh data. The StaticMesh asset is saved to
	 * /Game/ImportedAgxMeshes/'AssetFolderName' with the source name that the native
	 * agxCollide::Trimesh has. If it does not have a source name then 'ImportedAgxMesh' is used
	 * instead.
	 *
	 * @param Trimesh - AGX Dynamics trimesh data to convert to a StaticMesh.
	 * @param AssetFolderName - The name of the folder within /Game/ImportedAgxMeshes' that the
	 * asset should be stored to.
	 * @param FallbackName - Name used for the new Mesh Asset in case Trimesh does not have a
	 * source name.
	 */
	static UStaticMesh* CreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& AssetFolderName,
		const FString& FallbackName);

	/**
	 * Create a new UStaticMeshComponent. The UStaticMeshComponent will be added as a child to the
	 * given USceneComponent.
	 *
	 * @param Owner The Actor to which the StaticMeshComponent should be added.
	 * @param Outer The USceneComponent that the StaticMeshComponent should be a child of.
	 * @param MeshAsset The Mesh asset to assign the UStaticMeshComponent's static mesh to.
	 * @param bRegister True if AActor::RegisterComponent should be called. Must be called later if
	 * false.
	 */
	static UStaticMeshComponent* CreateStaticMeshComponent(
		AActor& Owner, USceneComponent& Outer, UStaticMesh& MeshAsset, bool bRegister);

	/**
	 * Creates a new UAGX_ShapeMaterialAsset for a shape material and returns the shape material
	 * asset path. Returns empty string if the asset could not be created.
	 */
	static FString CreateShapeMaterialAsset(
		const FString& DirName, const FShapeMaterialBarrier& Material);

	/**
	 * Creates a new UAGX_ContactMaterialAsset for a contact material and returns the contact
	 * material asset path. Returns empty string if the asset could not be created.
	 */
	static FString CreateContactMaterialAsset(
		const FString& DirName, const FContactMaterialBarrier& ContactMaterial,
		const FString& Material1, const FString& Material2);

	/**
	 * Create a new constraint of the specified type.
	 */
	static AAGX_ConstraintActor* CreateConstraintActor(
		UClass* ConstraintType, UAGX_RigidBodyComponent* RigidBody1,
		UAGX_RigidBodyComponent* RigidBody2, bool bSelect, bool bShowNotification,
		bool bInPlayingWorldIfAvailable);

	template <typename T>
	static T* CreateConstraintActor(
		UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2, bool bSelect,
		bool bShowNotification, bool bInPlayingWorldIfAvailable, UClass* ConstraintType = nullptr);

	static UAGX_ConstraintComponent* CreateConstraintComponent(
		AActor* Owner, UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2,
		UClass* ConstraintType = nullptr);

	template <typename UConstraint>
	static UConstraint* CreateConstraintComponent(
		AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2 = nullptr,
		UClass* ConstraintType = nullptr);

	static UAGX_HingeConstraintComponent* CreateHingeConstraintComponent(
		AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2);

	static UAGX_PrismaticConstraintComponent* CreatePrismaticConstraintComponent(
		AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2);

	/**
	 * Create a new AGX Constraint Frame Actor. Set as child to specified Rigid Body, if available.
	 */
	static AAGX_ConstraintFrameActor* CreateConstraintFrameActor(
		AActor* ParentActor, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable);

	/**
	 * Change current selection to the specific actor.
	 */
	static void SelectActor(AActor* Actor, bool DeselectPrevious = true);

	/**
	 * Display a temporary notification popup in the lower right corner of the Editor.
	 */
	static void ShowNotification(const FText& Text);

	/**
	 * Return the Editor world, i.e. not the one that is potentially currently playing.
	 */
	static UWorld* GetEditorWorld();

	/**
	 * Return the currently playing world, or null if game is not playing.
	 *
	 * @remarks Playing world can be access directly from actors, using the GetWorld()
	 * function which returns the world they belong in. Therefore this function should
	 * typically only be used by Editor code that does not have direct access to an Actor.
	 */
	static UWorld* GetPlayingWorld();

	/**
	 * Return GetPlayingWorld if game is playing, else returns GetEditorWorld.
	 *
	 * @remarks Current world can be access directly from actors, using the GetWorld()
	 * function which returns the world they belong in. Therefore this function should
	 * typically only be used by Editor code that does not have direct access to an Actor.
	 */
	static UWorld* GetCurrentWorld();

	/**
	 * Find the first two found actors that has a UAGX_RigidBodyComponent in the current selection,
	 * with options to search children and parents in case the user has selected a graphics-only
	 * actor instead of the actual rigid body actor.
	 *
	 * @param bSearchSubtrees If true, each selected actor's subtree will also be searched. But,
	 * once a Rigid Body is found, the search of the corresponding selected actor's subtree will be
	 * finished, such that only one Rigid Body actor may be found per selected actor.
	 *
	 * @param bSearchAncestors If true, all ancestors of each selected actor's subtree will also be
	 * searched.
	 *
	 */
	static void GetRigidBodyActorsFromSelection(
		AActor** OutActor1, AActor** OutActor2, bool bSearchSubtrees, bool bSearchAncestors);

	static AActor* GetRigidBodyActorFromSubtree(AActor* SubtreeRoot, const AActor* IgnoreActor);

	static AActor* GetRigidBodyActorFromAncestors(AActor* Actor, const AActor* IgnoreActor);

	static void GetAllClassesOfType(
		TArray<UClass*>& OutMatches, UClass* BaseClass, bool bIncludeAbstract);

	/**
	 * Returns single object being customized from DetailBuilder if found.
	 *
	 * @param FailIfMultiple If true, nullptr is returned if multiple objects are found.
	 * If False, the first found object is returned, even if multiple objects are found.
	 */
	template <typename T>
	static T* GetSingleObjectBeingCustomized(
		IDetailLayoutBuilder& DetailBuilder, bool FailIfMultiple = true);

	/**
	 * Assigns a shape material asset to the physical material of a UAGX_ShapeComponent, if the
	 * asset is found. The ShapeMaterialAsset FString contains the path to the asset along with its
	 * name.
	 */
	static bool ApplyShapeMaterial(UAGX_ShapeComponent* Shape, const FString& ShapeMaterialAsset);

	/**
	 * Convert a bool to a Slate visibility flag. True means visible, false means Collapsed (not
	 * Hidden).
	 */
	static EVisibility VisibleIf(bool bVisible);

	static FString SelectExistingFileDialog(
		const FString& FileDescription, const FString& FileExtension);

	static FString SelectExistingDirectoryDialog(
		const FString& DialogTitle, const FString& InStartDir = "", bool AllowNoneSelected = false);

	static FString SelectNewFileDialog(
		const FString& DialogTitle, const FString& FileExtension, const FString& FileTypes,
		const FString& DefaultFile = "", const FString& InStartDir = "");
};

template <typename T>
T* FAGX_EditorUtilities::CreateConstraintActor(
	UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2, bool bSelect,
	bool bShowNotification, bool bInPlayingWorldIfAvailable, UClass* ConstraintType)
{
	if (ConstraintType == nullptr)
	{
		ConstraintType = T::StaticClass();
	}
	check(ConstraintType->IsChildOf<T>());
	return Cast<T>(CreateConstraintActor(
		ConstraintType, RigidBody1, RigidBody2, bSelect, bShowNotification,
		bInPlayingWorldIfAvailable));
}

template <typename UConstraint>
UConstraint* FAGX_EditorUtilities::CreateConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2, UClass* Type)
{
	if (Type == nullptr)
	{
		Type = UConstraint::StaticClass();
	}
	UAGX_ConstraintComponent* BaseConstraint = CreateConstraintComponent(Owner, Body1, Body2, Type);
	if (BaseConstraint == nullptr)
	{
		return nullptr;
	}
	UConstraint* DerivedConstraint = Cast<UConstraint>(BaseConstraint);
	// The created constraint must be of the type we asked for.
	check(DerivedConstraint != nullptr);
	return DerivedConstraint;
}

template <typename T>
T* FAGX_EditorUtilities::GetSingleObjectBeingCustomized(
	IDetailLayoutBuilder& DetailBuilder, bool FailIfMultiple)
{
	static_assert(std::is_base_of<UObject, T>::value, "T must inherit from UObject");

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 1 || (!FailIfMultiple && Objects.Num() > 1))
		return Cast<T>(Objects[0].Get());
	else
		return nullptr;
}
