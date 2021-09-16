#include "Shapes/AGX_CylinderShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_UpropertyDispatcher.h"
#include "Utilities/AGX_MeshUtilities.h"

UAGX_CylinderShapeComponent::UAGX_CylinderShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Height = 100.0f;
	Radius = 50.0f;
}

void UAGX_CylinderShapeComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}

	Radius = InRadius;
	UpdateVisualMesh();
}

float UAGX_CylinderShapeComponent::GetRadius() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRadius();
	}

	return Radius;
}

void UAGX_CylinderShapeComponent::SetHeight(float InHeight)
{
	if (HasNative())
	{
		NativeBarrier.SetHeight(InHeight);
	}

	Height = InHeight;
	UpdateVisualMesh();
}

float UAGX_CylinderShapeComponent::GetHeight() const
{
	if (HasNative())
	{
		return NativeBarrier.GetHeight();
	}

	return Height;
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

FShapeBarrier* UAGX_CylinderShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CylinderShapeComponent::GetNativeBarrier() const
{
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

	NativeBarrier.SetHeight(Height * GetComponentScale().Y);
	NativeBarrier.SetRadius(Radius * GetComponentScale().X);
}

void UAGX_CylinderShapeComponent::CopyFrom(const FCylinderShapeBarrier& Barrier)
{
	Super::CopyFrom(Barrier);
	Height = Barrier.GetHeight();
	Radius = Barrier.GetRadius();
}

void UAGX_CylinderShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	const uint32 NumCircleSegments = 32;
	const uint32 NumHeightSegments = 1;

	AGX_MeshUtilities::MakeCylinder(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		AGX_MeshUtilities::CylinderConstructionData(
			Radius, Height, NumCircleSegments, NumHeightSegments));
}

#if WITH_EDITOR

bool UAGX_CylinderShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Note: radius and height are intentionally ignored here since the SetRadius / SetHeight
	// functions are responsible to call the UpdateVisualMesh. This is done since calling e.g.
	// SetRadius from a Blueprint will NOT trigger the PostEditChangeProperty where the
	// UpdateVisualMesh is usually called from.
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName);
}

#endif

void UAGX_CylinderShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_CylinderShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

#if WITH_EDITOR
void UAGX_CylinderShapeComponent::PostLoad()
{
	Super::PostLoad();
	InitPropertyDispatcher();
}

void UAGX_CylinderShapeComponent::InitPropertyDispatcher()
{
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, Radius),
		[](ThisClass* This) { This->SetRadius(This->Radius); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, Height),
		[](ThisClass* This) { This->SetHeight(This->Height); });
}

void UAGX_CylinderShapeComponent::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	// The root property that contains the property that was changed.
	const FName Member = (PropertyChangedEvent.MemberProperty != NULL)
							 ? PropertyChangedEvent.MemberProperty->GetFName()
							 : NAME_None;

	// The leaf property that was changed. May be nested in a struct.
	const FName Property = (PropertyChangedEvent.Property != NULL)
							   ? PropertyChangedEvent.Property->GetFName()
							   : NAME_None;

	if (FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(Member, Property, this))
	{
		// No custom handling required when handled by PropertyDispatcher callback.
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}

	// Add any custom property edited handling that may be required in the future here.

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_CylinderShapeComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.Num() < 3)
	{
		Super::PostEditChangeChainProperty(PropertyChangedEvent);

		// These simple cases are handled by PostEditChangeProperty, which is called by UObject's
		// PostEditChangeChainProperty.
		return;
	}

	FEditPropertyChain::TDoubleLinkedListNode* Node = PropertyChangedEvent.PropertyChain.GetHead();
	FName Member = Node->GetValue()->GetFName();
	Node = Node->GetNextNode();
	FName Property = Node->GetValue()->GetFName();
	// The name of the rest of the nodes doesn't matter, we set all elements at level two each
	// time. These are small objects such as FVector or FFloatInterval.
	// Some rewrite of FAGX_PropertyDispatcher will be required to support other types of nesting
	FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(Member, Property, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif
