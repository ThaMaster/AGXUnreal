#include "Shapes/AGX_CapsuleShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_UpropertyDispatcher.h"
#include "Utilities/AGX_MeshUtilities.h"

UAGX_CapsuleShapeComponent::UAGX_CapsuleShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Height = 100.0f;
	Radius = 50.0f;
}

void UAGX_CapsuleShapeComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}

	Radius = InRadius;
	UpdateVisualMesh();
}

float UAGX_CapsuleShapeComponent::GetRadius() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRadius();
	}

	return Radius;
}

void UAGX_CapsuleShapeComponent::SetHeight(float InHeight)
{
	if (HasNative())
	{
		NativeBarrier.SetHeight(InHeight);
	}

	Height = InHeight;
	UpdateVisualMesh();
}

float UAGX_CapsuleShapeComponent::GetHeight() const
{
	if (HasNative())
	{
		return NativeBarrier.GetHeight();
	}

	return Height;
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

FShapeBarrier* UAGX_CapsuleShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CapsuleShapeComponent::GetNativeBarrier() const
{
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
	// Note: radius and height are intentionally ignored here since the SetRadius / SetHeight
	// functions are responsible to call the UpdateVisualMesh. This is done since calling e.g.
	// SetRadius from a Blueprint will NOT trigger the PostEditChangeProperty where the
	// UpdateVisualMesh is usually called from.
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName);
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

#if WITH_EDITOR

void UAGX_CapsuleShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// Cannot use the UAGX_ShapeComponent Property Dispatcher because there are name collisions for
	// Shape-specific UProperty names, for example Radius is in both Sphere and Cylinder.
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CapsuleShapeComponent, Radius),
		[](ThisClass* This) { This->SetRadius(This->Radius); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CapsuleShapeComponent, Height),
		[](ThisClass* This) { This->SetHeight(This->Height); });
}

void UAGX_CapsuleShapeComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& Event)
{
	FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(Event, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif
