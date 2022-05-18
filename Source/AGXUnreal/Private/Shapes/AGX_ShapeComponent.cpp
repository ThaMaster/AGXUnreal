// Copyright 2022, Algoryx Simulation AB.

#include "Shapes/AGX_ShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Contacts/AGX_ShapeContact.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_TextureUtilities.h"

// Unreal Engine includes.
#include "Materials/Material.h"
#include "Misc/EngineVersionComparison.h"

// Sets default values for this component's properties
UAGX_ShapeComponent::UAGX_ShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ShapeComponent::HasNative() const
{
	return GetNative() != nullptr;
}

uint64 UAGX_ShapeComponent::GetNativeAddress() const
{
	return static_cast<uint64>(GetNativeBarrier()->GetNativeAddress());
}

void UAGX_ShapeComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	GetNativeBarrier()->SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

TStructOnScope<FActorComponentInstanceData> UAGX_ShapeComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_ShapeComponent::UpdateVisualMesh()
{
	ClearMeshData();

	TSharedPtr<FAGX_SimpleMeshData> Data(new FAGX_SimpleMeshData());

	if (ShouldCreateVisualMesh())
	{
		CreateVisualMesh(*Data.Get());
	}

	SetMeshData(Data);
}

bool UAGX_ShapeComponent::ShouldCreateVisualMesh() const
{
	/// \todo add && !(bHiddenInGame && IsGamePlaying), but how to get IsGamePlaying?
	return IsVisible();
}

void UAGX_ShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	GetNative()->SetName(GetName());
	GetNative()->SetIsSensor(bIsSensor, SensorType == EAGX_ShapeSensorType::ContactsSensor);

	if (ShapeMaterial)
	{
		UAGX_ShapeMaterialInstance* MaterialInstance = static_cast<UAGX_ShapeMaterialInstance*>(
			ShapeMaterial->GetOrCreateInstance(GetWorld()));
		check(MaterialInstance);
		UWorld* PlayingWorld = GetWorld();
		if (MaterialInstance != ShapeMaterial && PlayingWorld && PlayingWorld->IsGameWorld())
		{
			ShapeMaterial = MaterialInstance;
		}
		FShapeMaterialBarrier* MaterialBarrier =
			MaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
		check(MaterialBarrier);
		GetNative()->SetMaterial(*MaterialBarrier);
	}

	GetNative()->SetEnableCollisions(bCanCollide);

	for (const FName& Group : CollisionGroups)
	{
		GetNative()->AddCollisionGroup(Group);
	}
}

#if WITH_EDITOR

bool UAGX_ShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
#if UE_VERSION_OLDER_THAN(4, 24, 0)
	const FName& VisibleName = GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bVisible);
	const FName& RelativeScale3DName =
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, RelativeScale3D);
#else
	/// \todo bVisible and RelativeScale3D will become private in some Unreal Engine version > 4.25.
	/// Unclear how we should handle PostEditChangeProperty events for those after that, since
	/// GET_MEMBER_NAME_CHECKED can't be used with inherited private properties.
	/// Monitor
	/// https://answers.unrealengine.com/questions/950031/how-to-use-get-member-name-checked-with-upropertie.html
	/// for possible solutions.
	FName VisibleName(TEXT("bVisible"));
	FName RelativeScale3DName(TEXT("RelativeScale3D"));
#endif
	return PropertyName == VisibleName || MemberPropertyName == RelativeScale3DName;
}

void UAGX_ShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = GetFNameSafe(PropertyChangedEvent.Property);
	const FName MemberPropertyName = GetFNameSafe(PropertyChangedEvent.MemberProperty);

	if (DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName))
	{
		UpdateVisualMesh();
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_ShapeComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif

void UAGX_ShapeComponent::PostLoad()
{
	Super::PostLoad();
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateVisualMesh();

#if WITH_EDITOR
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bCanCollide),
		[](ThisClass* This) { This->SetCanCollide(This->bCanCollide); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bIsSensor),
		[](ThisClass* This) { This->SetIsSensor(This->bIsSensor); });
#endif
}

void UAGX_ShapeComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GIsReconstructingBlueprintInstances)
	{
		// This Component will soon be given a Native Geometry and Shape from a
		// FAGX_NativeOwnerInstanceData, so don't create a new one here.
		return;
	}

	GetOrCreateNative();
	UAGX_RigidBodyComponent* RigidBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
	if (RigidBody == nullptr)
	{
		// This shape doesn't have a parent body so the native shape's local transform will become
		// its world transform. Push the entire Unreal world transform down into the native shape.
		UpdateNativeGlobalTransform();
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shape '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_ShapeComponent will inherit this one's Native, so don't wreck it.
		// It's still safe to release the native since the Simulation will hold a reference if
		// necessary.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor && Reason != EEndPlayReason::Quit)
	{
		// @todo Figure out how to handle removal of Shape Materials from the Simulation. They can
		// be shared between many Shape Components, so some kind of reference counting might be
		// needed.
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		ReleaseNative();
	}
}

void UAGX_ShapeComponent::CopyFrom(const FShapeBarrier& Barrier)
{
	bCanCollide = Barrier.GetEnableCollisions();
	bIsSensor = Barrier.GetIsSensor();
	SensorType = Barrier.GetIsSensorGeneratingContactData() ? EAGX_ShapeSensorType::ContactsSensor
															: EAGX_ShapeSensorType::BooleanSensor;

	FVector Position;
	FQuat Rotation;
	std::tie(Position, Rotation) = Barrier.GetLocalPositionAndRotation();
	SetRelativeLocationAndRotation(Position, Rotation);

	TArray<FName> NewCollisionGroups = Barrier.GetCollisionGroups();
	CollisionGroups.Empty(NewCollisionGroups.Num());
	for (const FName& Group : NewCollisionGroups)
	{
		AddCollisionGroup(Group);
	}

	/// \todo Should shape material be handled here? If so, how? We don't have access to the
	/// <Guid, Object> restore tables from here.
}

void UAGX_ShapeComponent::UpdateNativeGlobalTransform()
{
	check(HasNative());
	FShapeBarrier* Shape = GetNative();
	Shape->SetLocalPosition(GetComponentLocation());
	Shape->SetLocalRotation(GetComponentQuat());
}

void UAGX_ShapeComponent::AddCollisionGroup(const FName& GroupName)
{
	if (!GroupName.IsNone())
		CollisionGroups.AddUnique(GroupName);
}

void UAGX_ShapeComponent::RemoveCollisionGroupIfExists(const FName& GroupName)
{
	if (!GroupName.IsNone())
	{
		auto Index = CollisionGroups.IndexOfByKey(GroupName);

		if (Index != INDEX_NONE)
		{
			CollisionGroups.RemoveAt(Index);
		}
	}
}

bool UAGX_ShapeComponent::SetShapeMaterial(UAGX_ShapeMaterialBase* InShapeMaterial)
{
	if (InShapeMaterial == nullptr)
	{
		if (HasNative())
		{
			GetNative()->ClearMaterial();
		}
		ShapeMaterial = nullptr;
		return true;
	}

	if (!HasNative())
	{
		// Not initialized yet, so simply assign the material we're given.
		ShapeMaterial = InShapeMaterial;
		return true;
	}

	// This Shape has already been initialized. Use the Instance version of the material, which
	// may be ShapeMaterial itself.
	UWorld* World = GetWorld();
	UAGX_ShapeMaterialInstance* Instance =
		static_cast<UAGX_ShapeMaterialInstance*>(InShapeMaterial->GetOrCreateInstance(World));
	if (Instance == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shape '%s', in Actor '%s', could not create Shape Material Instance for '%s'. "
				 "Material not changed."),
			*GetName(), *GetLabelSafe(GetOwner()), *InShapeMaterial->GetName());
		return false;
	}
	ShapeMaterial = Instance;

	// Assign the new native Material to the native Shape.
	FShapeMaterialBarrier* NativeMaterial = Instance->GetOrCreateShapeMaterialNative(World);
	if (NativeMaterial == nullptr || !NativeMaterial->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create AGX Dynamics representation of Shape Material '%s' when "
				 "assigned to Shape '%s' in Actor '%s'."),
			*InShapeMaterial->GetName(), *GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}
	GetNative()->SetMaterial(*NativeMaterial);

	return true;
}

void UAGX_ShapeComponent::SetCanCollide(bool CanCollide)
{
	if (HasNative())
	{
		GetNative()->SetEnableCollisions(CanCollide);
	}

	bCanCollide = CanCollide;
}

bool UAGX_ShapeComponent::GetCanCollide() const
{
	if (HasNative())
	{
		return GetNative()->GetEnableCollisions();
	}

	return bCanCollide;
}

void UAGX_ShapeComponent::SetIsSensor(bool IsSensor)
{
	if (HasNative())
	{
		GetNative()->SetIsSensor(IsSensor, SensorType == EAGX_ShapeSensorType::ContactsSensor);
	}

	bIsSensor = IsSensor;

	IsSensor ? ApplySensorMaterial(*this) : RemoveSensorMaterial(*this);
}

bool UAGX_ShapeComponent::GetIsSensor() const
{
	if (HasNative())
	{
		return GetNative()->GetIsSensor();
	}

	return bIsSensor;
}

TArray<FAGX_ShapeContact> UAGX_ShapeComponent::GetShapeContacts() const
{
	if (!HasNative())
	{
		return TArray<FAGX_ShapeContact>();
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (!Simulation)
	{
		return TArray<FAGX_ShapeContact>();
	}

	TArray<FShapeContactBarrier> Barriers = Simulation->GetShapeContacts(*GetNative());
	TArray<FAGX_ShapeContact> ShapeContacts;
	ShapeContacts.Reserve(Barriers.Num());
	for (FShapeContactBarrier& Barrier : Barriers)
	{
		ShapeContacts.Emplace(std::move(Barrier));
	}
	return ShapeContacts;
}

void UAGX_ShapeComponent::ApplySensorMaterial(UMeshComponent& Mesh)
{
	static const TCHAR* AssetPath =
		TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'");
	static UMaterial* SensorMaterial = FAGX_TextureUtilities::GetMaterialFromAssetPath(AssetPath);
	if (SensorMaterial == nullptr)
	{
		return;
	}

	for (int32 I = 0; I < Mesh.GetNumMaterials(); ++I)
	{
		// Don't want to ruin the material setup of any mesh that has one so only setting the
		// sensor material to empty material slots. The intention is that sensors usually aren't
		// rendered at all in-game so their material slots should be empty.
		if (Mesh.GetMaterial(I) != nullptr)
		{
			continue;
		}
		Mesh.SetMaterial(I, SensorMaterial);
	}
}

void UAGX_ShapeComponent::RemoveSensorMaterial(UMeshComponent& Mesh)
{
	for (int32 I = 0; I < Mesh.GetNumMaterials(); ++I)
	{
		const UMaterialInterface* const Material = Mesh.GetMaterial(I);
		if (Material == nullptr)
		{
			continue;
		}
		if (Material->GetName() != TEXT("M_SensorMaterial"))
		{
			continue;
		}
		Mesh.SetMaterial(I, nullptr);
	}
}

void UAGX_ShapeComponent::OnUpdateTransform(
	EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	if (UpdateTransformFlags == EUpdateTransformFlags::PropagateFromParent)
	{
		return;
	}

	if (!HasNative())
	{
		return;
	}

	// This Shape's transform was updated and it was not because of a parent moving. This should be
	// propagated to the native to keep the AGX Dynamics state in line with the Unreal state.
	// This can for example be due to the user manually changing the transform of the Shape during
	// play by dragging it around in the world, or setting new values via the Details Panel or
	// Blueprint functions. This also covers cases where the Shape is detached from a parent
	// Component and its transform is changed. One exception to this is when the detachment is done
	// from a Blueprint using the DetachFromComponent function with Keep World settings. In that
	// case his function is not triggered with EUpdateTransformFlags != PropagateFromParent until
	// this Shape Component is selected (highlighted) in the Details Panel. Unclear why. This
	// corner-case is instead directly handled in UAGX_ShapeComponent::OnAttachmentChanged.
	GetNative()->SetWorldPosition(GetComponentLocation());
	GetNative()->SetWorldRotation(GetComponentQuat());
}

void UAGX_ShapeComponent::OnAttachmentChanged()
{
	if (!HasNative())
	{
		return;
	}
	// This is somewhat of a hack, but it works. This Shape's parent has changed, so this
	// Unreal object might get a new world transform depending on how it was detached.  Ideally,
	// this would be fully handled by UAGX_ShapeComponent::OnUpdateTransform, but that function is
	// not triggered if the detachment was made using the Blueprint function DetachFromComponent
	// with Keep World settings. Therefore we handle that here. Note that this will mean writing to
	// the native twice when detaching in other ways (both this function and OnUpdateTransform is
	// triggered). However, setting the world transform of this Component to the natives world
	// transform is generally considered safe since they should always be in sync.
	GetNative()->SetWorldPosition(GetComponentLocation());
	GetNative()->SetWorldRotation(GetComponentQuat());
}

void UAGX_ShapeComponent::OnRegister()
{
	Super::OnRegister();
	UpdateVisualMesh();
}
