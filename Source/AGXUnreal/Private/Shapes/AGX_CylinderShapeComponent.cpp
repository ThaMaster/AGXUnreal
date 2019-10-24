#include "Shapes/AGX_CylinderShapeComponent.h"

#include "AGX_LogCategory.h"
#include "AGX_MeshUtilities.h"


UAGX_CylinderShapeComponent::UAGX_CylinderShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Height = 100.0f;
	Radius = 50.0f;
	UE_LOG(LogAGX, Log, TEXT("CylinderShape instance created."));
}

FShapeBarrier* UAGX_CylinderShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CylinderShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_CylinderShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FCylinderShapeBarrier* UAGX_CylinderShapeComponent::GetNativeCylinder()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_CylinderShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();
	
	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetHeight(Height * GetComponentScale().Y, GetWorld());
	NativeBarrier.SetRadius(Radius * GetComponentScale().X, GetWorld());
}

void UAGX_CylinderShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& MeshData)
{
	const uint32 NumCircleSegments = 32;
	const uint32 NumHeightSegments = 1;

	AGX_MeshUtilities::MakeCylinder(MeshData.Vertices, MeshData.Normals, MeshData.Indices, Radius, Height, NumCircleSegments, NumHeightSegments);
}

#if WITH_EDITOR

bool UAGX_CylinderShapeComponent::DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const
{
	return 
		Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, Height) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, Radius);
}

#endif

void UAGX_CylinderShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for CylinderShapeComponent."));
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_CylinderShapeComponent::ReleaseNative()
{
	UE_LOG(LogAGX, Log, TEXT("Releasing native object for CylinderShapeComponent."));
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
