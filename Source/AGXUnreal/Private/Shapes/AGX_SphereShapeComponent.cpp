#include "Shapes/AGX_SphereShapeComponent.h"

#include "AGX_LogCategory.h"

UAGX_SphereShapeComponent::UAGX_SphereShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Radius = 50.0f;
	UE_LOG(LogAGX, Log, TEXT("SphereShape instance created."));
}

FShapeBarrier* UAGX_SphereShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_SphereShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_SphereShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FSphereShapeBarrier* UAGX_SphereShapeComponent::GetNativeSphere()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_SphereShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetRadius(Radius * GetComponentScale().X, GetWorld());
}

void UAGX_SphereShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& MeshData)
{
	AGX_MeshUtilities::MakeSphere(MeshData.Vertices, MeshData.Normals, MeshData.Indices, Radius, 32);
}

#if WITH_EDITOR

bool UAGX_SphereShapeComponent::DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const
{
	return
		Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UAGX_SphereShapeComponent, Radius);
}

#endif

void UAGX_SphereShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for SphereShapeComponent."));
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_SphereShapeComponent::ReleaseNative()
{
	UE_LOG(LogAGX, Log, TEXT("Releasing native object for SphereShapeComponent."));
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
