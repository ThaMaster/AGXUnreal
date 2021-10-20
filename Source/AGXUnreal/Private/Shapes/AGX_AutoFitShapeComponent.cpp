#include "Shapes/AGX_AutoFitShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EnvironmentUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

namespace AGX_AutoFitShapeComponent_helpers
{
	void LogErrorWithMessageBoxInEditor(const FString& Msg, UWorld* World)
	{
		if (World && World->IsGameWorld())
		{
			// Write only to the log during Play.
			UE_LOG(LogAGX, Error, TEXT("%s"), *Msg);
		}
		else
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(Msg);
		}
	}

	void LogWarningWithMessageBoxInEditor(const FString& Msg, UWorld* World)
	{
		if (World && World->IsGameWorld())
		{
			// Write only to the log during Play.
			UE_LOG(LogAGX, Warning, TEXT("%s"), *Msg);
		}
		else
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithWarningLog(Msg);
		}
	}
}

bool UAGX_AutoFitShapeComponent::AutoFit(TArray<FAGX_MeshWithTransform> Meshes)
{
	if (!FAGX_EnvironmentUtilities::IsAGXDynamicsVersionNewerOrEqualTo(2, 31, 0, 0))
	{
		AGX_AutoFitShapeComponent_helpers::LogErrorWithMessageBoxInEditor(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes because the AGX Dynamics version used by "
					 "the AGX Dynamics for Unreal plugin is too old. The AGX Dynamics version must "
					 "be at least 2.31.0.0."),
				*GetName()),
			GetWorld());
		return false;
	}

	TArray<FVector> Vertices;
	int32 numWarnings = 0;
	for (const FAGX_MeshWithTransform& Mesh : Meshes)
	{
		TArray<FVector> MeshVertices;
		TArray<FTriIndices> MeshIndices;
		const bool CollisionDataResult = AGX_MeshUtilities::GetStaticMeshCollisionData(
			Mesh, FTransform::Identity, MeshVertices, MeshIndices);
		if (CollisionDataResult)
		{
			Vertices.Append(MeshVertices);
		}
		else
		{
			numWarnings++;
		}
	}
	if (Vertices.Num() == 0)
	{
		AGX_AutoFitShapeComponent_helpers::LogErrorWithMessageBoxInEditor(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes because no collision data could be "
					 "extracted."),
				*GetName()),
			GetWorld());
		return false;
	}
	else if (numWarnings > 0)
	{
		AGX_AutoFitShapeComponent_helpers::LogWarningWithMessageBoxInEditor(
			"At least one warning was detected during the auto-fit process. The Log may contain "
			"more details.",
			GetWorld());
	}

	const bool Result = AutoFitFromVertices(Vertices);
	if (!Result)
	{
		AGX_AutoFitShapeComponent_helpers::LogErrorWithMessageBoxInEditor(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes. The Log may contain more details."),
				*GetName()),
			GetWorld());
	}

	return Result;
}

bool UAGX_AutoFitShapeComponent::GetStaticMeshCollisionData(
	TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices) const
{
	FAGX_MeshWithTransform Mesh;
	switch (MeshSourceLocation)
	{
		case EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT:
			Mesh = AGX_MeshUtilities::FindFirstChildMesh(*this);
			break;
		case EAGX_StaticMeshSourceLocation::TSL_PARENT_STATIC_MESH_COMPONENT:
			Mesh = AGX_MeshUtilities::FindFirstParentMesh(*this);
			break;
		case EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET:
			if (MeshSourceAsset != nullptr)
			{
				Mesh = FAGX_MeshWithTransform(MeshSourceAsset, GetComponentTransform());
			}
			break;
	}

	if (!Mesh.IsValid())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetStaticMeshCollisionData failed for '%s'. Unable to find static Mesh."),
			*GetName());
		return false;
	}

	return AGX_MeshUtilities::GetStaticMeshCollisionData(
		Mesh, GetComponentTransform(), OutVertices, OutIndices);
}
