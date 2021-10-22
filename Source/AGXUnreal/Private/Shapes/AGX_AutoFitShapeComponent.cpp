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

	const bool Result = AutoFitFromVertices(Vertices);
	if (!Result)
	{
		AGX_AutoFitShapeComponent_helpers::LogErrorWithMessageBoxInEditor(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes. The Log may contain more details."),
				*GetName()),
			GetWorld());
		return false;
	}

	if (numWarnings > 0)
	{
		AGX_AutoFitShapeComponent_helpers::LogWarningWithMessageBoxInEditor(
			"At least one warning was detected during the auto-fit process. The Log may contain "
			"more details.",
			GetWorld());
	}

	return true;
}

bool UAGX_AutoFitShapeComponent::AutoFitFromSelection()
{
	// The TSL_CHILD_STATIC_MESH_COMPONENT is a special case where care must be taken to restore
	// the original world transform of the children components.
	if (MeshSourceLocation == EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT)
	{
		return AutoFitToChildrenFromSelection();
	}

	TArray<FAGX_MeshWithTransform> Meshes = GetSelectedStaticMeshes();
	return AutoFit(Meshes);
}

bool UAGX_AutoFitShapeComponent::AutoFitToChildrenFromSelection()
{
	// Store away the world transforms of the Static Mesh Components so that we can restore them
	// after the auto-fit procedure.
	TArray<UStaticMeshComponent*> ChildComponents =
		AGX_MeshUtilities::FindImmediateChildrenMeshComponents(*this);
	TMap<UStaticMeshComponent*, FTransform> OrigChildWorldTransforms;
	for (UStaticMeshComponent* S : ChildComponents)
	{
		if (S != nullptr)
		{
			OrigChildWorldTransforms.Add(S, S->GetComponentTransform());
			// Call Modify on the child component so that Undo/Redo works as expected.
			S->Modify();
		}
	}

	const bool Result = AutoFit(GetSelectedStaticMeshes());
	if (!Result)
	{
		return false;
	}

	// Finally, we restore the original world transform of the children Static Mesh components.
	for (UStaticMeshComponent* S : ChildComponents)
	{
		if (S != nullptr)
		{
			S->SetWorldTransform(OrigChildWorldTransforms[S]);
		}
	}

	return true;
}

TArray<FAGX_MeshWithTransform> UAGX_AutoFitShapeComponent::GetSelectedStaticMeshes() const
{
	TArray<FAGX_MeshWithTransform> Meshes;
	switch (MeshSourceLocation)
	{
		case EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT:
			Meshes = AGX_MeshUtilities::FindImmediateChildrenMeshes(*this);
			break;
		case EAGX_StaticMeshSourceLocation::TSL_PARENT_STATIC_MESH_COMPONENT:
			Meshes.Add(AGX_MeshUtilities::FindFirstParentMesh(*this));
			break;
		case EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET:
			if (MeshSourceAsset != nullptr)
			{
				Meshes.Add(FAGX_MeshWithTransform(MeshSourceAsset, GetComponentTransform()));
			}
			break;
	}

	return Meshes;
}
