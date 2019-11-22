#include "AGX_HeightFieldShapeComponent.h"

#include "AGX_HeightFieldUtilities.h"

#include "AGX_LogCategory.h"

UAGX_HeightFieldShapeComponent::UAGX_HeightFieldShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

};

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
};

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
	/// \todo More may be needed here.
}


void UAGX_HeightFieldShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& /*OutMeshData*/)
{
	/// \todo Not sure how to best handle this. The HeightField is, so far, only
	/// used for ULandscape/UAGX_Terrain.
}

#if WITH_EDITOR
bool UAGX_HeightFieldShapeComponent::DoesPropertyAffectVisualMesh(const FName& /*PropertyName*/, const FName& /*MemberPropertyName*/) const
{
	/// \todo More may be needed here in the future.
	return false;
}
#endif

void UAGX_HeightFieldShapeComponent::CreateNative()
{
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
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
