#include "Shapes/AGX_CapsuleShapeComponent.h"

#include "Utilities/AGX_MeshUtilities.h"

UAGX_CapsuleShapeComponent::UAGX_CapsuleShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Height = 100.0f;
	Radius = 50.0f;
}

FShapeBarrier* UAGX_CapsuleShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CapsuleShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_CapsuleShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FCapsuleShapeBarrier* UAGX_CapsuleShapeComponent::GetNativeCapsule()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_CapsuleShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetHeight(Height * GetComponentScale().Y);
	NativeBarrier.SetRadius(Radius * GetComponentScale().X);
}

void UAGX_CapsuleShapeComponent::CopyFrom(const FCapsuleShapeBarrier& Barrier)
{
	Super::CopyFrom(Barrier);
	Height = Barrier.GetHeight();
	Radius = Barrier.GetRadius();
}

void UAGX_CapsuleShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	const uint32 NumCircleSegments = 32;
	const uint32 NumHeightSegments = 1;

	AGX_MeshUtilities::MakeCapsule(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		AGX_MeshUtilities::CapsuleConstructionData(
			Radius, Height, NumCircleSegments, NumHeightSegments));
}

#if WITH_EDITOR

bool UAGX_CapsuleShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
		   MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_CapsuleShapeComponent, Height) ||
		   MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_CapsuleShapeComponent, Radius);
}

#endif

void UAGX_CapsuleShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_CapsuleShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
