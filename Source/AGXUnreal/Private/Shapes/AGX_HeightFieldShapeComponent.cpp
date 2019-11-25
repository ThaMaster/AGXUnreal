#include "AGX_HeightFieldShapeComponent.h"

#include "AGX_HeightFieldUtilities.h"

#include "AGX_LogCategory.h"
#include "Utilities/AGX_MeshUtilities.h"


UAGX_HeightFieldShapeComponent::UAGX_HeightFieldShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogAGX, Log, TEXT("BoxShape instance created."));
}

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FHeightFieldShapeBarrier* UAGX_HeightFieldShapeComponent::GetNativeHeightField()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_HeightFieldShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	/// \todo What is the height field equivalent of this?
	// NativeBarrier.SetHalfExtents(HalfExtent * GetComponentScale());
}

void UAGX_HeightFieldShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	/// \todo What is the height field equivalent of this?
	//AGX_MeshUtilities::MakeCube(OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, HalfExtent);
}

#if WITH_EDITOR

bool UAGX_HeightFieldShapeComponent::DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const
{
	return false;
	// return
	//		Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
	//		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_HeightFieldShapeComponent, HalfExtent);
}

#endif

void UAGX_HeightFieldShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for HeightFieldShapeComponent."));
	check(!HasNative());
	if (SourceLandscape == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("HeightFieldComponent hasn't been given a source Landscape. Will not be included in the simulation."));
		return;
	}

	NativeBarrier = AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	UpdateNativeProperties();
}

void UAGX_HeightFieldShapeComponent::ReleaseNative()
{
	UE_LOG(LogAGX, Log, TEXT("Releasing native object for HeightFieldShapeComponent."));
	check(HasNative());
	NativeBarrier.ReleaseNative();
}


