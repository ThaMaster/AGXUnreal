#include "Shapes/AGX_BoxShapeComponent.h"

#include "Utilities/AGX_MeshUtilities.h"

UAGX_BoxShapeComponent::UAGX_BoxShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	HalfExtent = FVector(50.0f, 50.0f, 50.0f);
}

FShapeBarrier* UAGX_BoxShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_BoxShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_BoxShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FBoxShapeBarrier* UAGX_BoxShapeComponent::GetNativeBox()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_BoxShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetHalfExtents(HalfExtent * GetComponentScale());
}

void UAGX_BoxShapeComponent::CopyFrom(const FBoxShapeBarrier& Barrier)
{
	Super::CopyFrom(Barrier);
	HalfExtent = Barrier.GetHalfExtents();
}

void UAGX_BoxShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	AGX_MeshUtilities::MakeCube(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		HalfExtent);
}

#if WITH_EDITOR

bool UAGX_BoxShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
		   MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_BoxShapeComponent, HalfExtent);
}

#endif

void UAGX_BoxShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_BoxShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
