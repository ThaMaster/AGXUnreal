// Copyright 2022, Algoryx Simulation AB.

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
	// HeightFields are tightly coupled to a source landscape. This component should always be
	// positioned so that heights in the height field align with heights in the source landscape.
	// This sets up a callback so that we can position this component when the source landscape is
	// moved.
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

	UpdateNativeGlobalTransform();

	/// \todo What is the height field equivalent of this?
	// NativeBarrier.SetHalfExtents(HalfExtent * GetComponentScale());
}

void UAGX_HeightFieldShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	/// \todo What is the height field equivalent of this?
	// AGX_MeshUtilities::MakeCube(OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices,
	// HalfExtent);
}

void UAGX_HeightFieldShapeComponent::UpdateNativeGlobalTransform()
{
	// We override this function because the parents class' version of it does not allow HasNative
	// to be false, which is common for a Hight Field Component.
	if (HasNative())
	{
		UAGX_ShapeComponent::UpdateNativeGlobalTransform();
	}
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

	RecenterOnLandscape();
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
	if (PropertyChangedEvent.Property->GetFName() != GetRelativeLocationPropertyName() &&
		PropertyChangedEvent.Property->GetFName() != GetRelativeRotationPropertyName())
	{
		return;
	}

	RecenterOnLandscape();
}
#endif

void UAGX_HeightFieldShapeComponent::RecenterOnLandscape()
{
	// This function places this component at the center of the Landscape such that it aligns
	// correctly for the Native Geometry holding the Height Field.
	check(SourceLandscape != nullptr);

	std::tuple<FVector, FQuat> PosRot =
		AGX_HeightFieldUtilities::GetHeightFieldPositionAndRotationFrom(*SourceLandscape);
	SetWorldLocation(std::get<0>(PosRot));
	SetWorldRotation(std::get<1>(PosRot));
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

#if UE_VERSION_OLDER_THAN(5, 0, 0) == false
	const bool IsOpenWorldLandscape = SourceLandscape->LandscapeComponents.Num() <= 0;
	if (IsOpenWorldLandscape)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Attempted to use AGX Terrain with an Open World Landscape. Open "
				 "World Landscapes are currently not supported. Please use a non Open World "
				 "Level."));
		return;
	}
#endif

	NativeBarrier = AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	check(HasNative());
	UpdateNativeProperties();
}

void UAGX_HeightFieldShapeComponent::ReleaseNative()
{
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}
