#include "Shapes/AGX_HeightFieldShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "Misc/EngineVersionComparison.h"

UAGX_HeightFieldShapeComponent::UAGX_HeightFieldShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

#if WITH_EDITOR
	// HeightFields are tightly coupled to a source landscape. The Actor owning
	// this HeightField should always be positioned so that heights in the
	// height field align with heights in the source landscape. This sets up a
	// callback so that we can position the owning actor when the source
	// landscape is moved.
	//
	/// \todo This setup will call the callback for changes in ALL properties on
	/// ALL objects. That seems a bit wasteful. Find a way to bind narrower.
	OnPropertyChangedHandle =
		FCoreUObjectDelegates::FOnObjectPropertyChanged::FDelegate::CreateUObject(
			this, &UAGX_HeightFieldShapeComponent::OnSourceLandscapeChanged);
	OnPropertyChangedHandleDelegateHandle =
		FCoreUObjectDelegates::OnObjectPropertyChanged.Add(OnPropertyChangedHandle);
#endif
}

UAGX_HeightFieldShapeComponent::~UAGX_HeightFieldShapeComponent()
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedHandleDelegateHandle);
#endif
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

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNativeBarrier() const
{
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

void UAGX_HeightFieldShapeComponent::CopyFrom(const FHeightFieldShapeBarrier& Barrier)
{
	Super::CopyFrom(Barrier);
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
	// AGX_MeshUtilities::MakeCube(OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices,
	// HalfExtent);
}

#if WITH_EDITOR
void UAGX_HeightFieldShapeComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	static const FName PropertyNameSourceLandscape =
		GET_MEMBER_NAME_STRING_CHECKED(UAGX_HeightFieldShapeComponent, SourceLandscape);

	Super::PostEditChangeChainProperty(PropertyChangedEvent);

#if UE_VERSION_OLDER_THAN(4, 25, 0)
	UProperty* const ChangedProperty = PropertyChangedEvent.Property;
#else
	FProperty* const ChangedProperty = PropertyChangedEvent.Property;
#endif

	if (ChangedProperty == nullptr)
	{
		return;
	}

	if (ChangedProperty->GetFName() != PropertyNameSourceLandscape)
	{
		return;
	}

	if (SourceLandscape == nullptr)
	{
		return;
	}

	RecenterActorOnLandscape();
}
#endif

#if WITH_EDITOR
void UAGX_HeightFieldShapeComponent::OnSourceLandscapeChanged(
	UObject* SomeObject, struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Some of these checks are required because I don't know how to bind
	// property callbacks to a single property of a single object.
	if (SomeObject != SourceLandscape)
	{
		return;
	}
	if (SourceLandscape == nullptr)
	{
		return;
	}
	if (PropertyChangedEvent.Property->GetFName() != TEXT("RelativeLocation"))
	{
		return;
	}

	RecenterActorOnLandscape();
}
#endif

void UAGX_HeightFieldShapeComponent::RecenterActorOnLandscape()
{
	check(SourceLandscape != nullptr);

	// Assumes that the Landscape is rectangular and uniform, and that the actor
	// location is in the lower left corner of the component grid, i.e., that
	// components are laid out along positive X and Y.
	//
	// AGX Dynamics height fields have their model origin at the center of the
	// height field.
	FAGX_LandscapeSizeInfo LandscapeSizeInfo(*SourceLandscape);

	const float SideSizeX = LandscapeSizeInfo.NumQuadsSideX * LandscapeSizeInfo.QuadSideSizeX;
	const float SideSizeY = LandscapeSizeInfo.NumQuadsSideY * LandscapeSizeInfo.QuadSideSizeY;

	const FVector Location = SourceLandscape->GetActorLocation();
	const FVector Middle = Location + FVector(SideSizeX / 2.0f, SideSizeY / 2.0f, Location.Z);
	GetOwner()->SetActorLocation(Middle);
	MarkRenderStateDirty(); /// \todo Not sure if this is actually required or not.
}

#if WITH_EDITOR

bool UAGX_HeightFieldShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Height fields does not have a visual mesh, so there there is nothing to affect.
	return false;
}

#endif

void UAGX_HeightFieldShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for HeightFieldShapeComponent."));
	check(!HasNative());
	if (SourceLandscape == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("HeightFieldComponent hasn't been given a source Landscape. Will not be included "
				 "in the simulation."));
		return;
	}

	NativeBarrier = AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	check(HasNative());
	UpdateNativeProperties();
}

void UAGX_HeightFieldShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
