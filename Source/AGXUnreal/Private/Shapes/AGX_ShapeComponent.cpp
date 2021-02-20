#include "Shapes/AGX_ShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
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
	GetNative()->SetIsSensor(bIsSensor, SensorType == EAGX_ShapeSensorType::ContactSensor);

	if (PhysicalMaterial)
	{
		UAGX_ShapeMaterialInstance* MaterialInstance = static_cast<UAGX_ShapeMaterialInstance*>(
			PhysicalMaterial->GetOrCreateInstance(GetWorld()));
		check(MaterialInstance);
		UWorld* PlayingWorld = GetWorld();
		if (MaterialInstance != PhysicalMaterial && PlayingWorld && PlayingWorld->IsGameWorld())
		{
			PhysicalMaterial = MaterialInstance;
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
	return PropertyName == VisibleName || PropertyName == RelativeScale3DName;
}

void UAGX_ShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = GetFNameSafe(PropertyChangedEvent.Property);
	FName MemberPropertyName = GetFNameSafe(PropertyChangedEvent.MemberProperty);

	if (DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName))
	{
		UpdateVisualMesh();
	}

	// @todo Follow the below pattern for all relevant UPROPERTIES to support live changes from
	// the details panel in the Editor during play. Note that setting a UPROPERTY from c++ does not
	// trigger the PostEditChangeProperty(), so no recursive loops will occur.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bCanCollide))
	{
		SetCanCollide(bCanCollide);
		return;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bIsSensor))
	{
		SetIsSensor(bIsSensor);
		return;
	}
}

void UAGX_ShapeComponent::PostLoad()
{
	Super::PostLoad();

	UpdateVisualMesh();
}

void UAGX_ShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();

	UpdateVisualMesh();
}

void UAGX_ShapeComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	UpdateVisualMesh();
}

#endif

void UAGX_ShapeComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOrCreateNative();
	UAGX_RigidBodyComponent* RigidBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
	if (RigidBody == nullptr)
	{
		// This shape doesn't have a parent body so the native shape's local transform will become
		// its world transform. Push the entire Unreal world transform down into the native shape.
		UpdateNativeGlobalTransform();

		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
		Simulation->AddShape(this);
	}
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	ReleaseNative();
}

void UAGX_ShapeComponent::CopyFrom(const FShapeBarrier& Barrier)
{
	bCanCollide = Barrier.GetEnableCollisions();
	bIsSensor = Barrier.GetIsSensor();
	SensorType = Barrier.GetIsSensorGeneratingContactData() ? EAGX_ShapeSensorType::ContactSensor
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
		GetNative()->SetIsSensor(IsSensor, SensorType == EAGX_ShapeSensorType::ContactSensor);
	}

	bIsSensor = IsSensor;
}

bool UAGX_ShapeComponent::GetIsSensor() const
{
	if (HasNative())
	{
		return GetNative()->GetIsSensor();
	}

	return bIsSensor;
}

TArray<FAGX_SensorContact> UAGX_ShapeComponent::GetSensorContacts()
{
	TArray<FAGX_SensorContact> SensorContacts;
	if (!HasNative())
	{
		return SensorContacts;
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (!Simulation)
	{
		return SensorContacts;
	}

	for (FSensorContactData& Data : Simulation->GetSensorContactData(*GetNative()))
	{
		SensorContacts.Add(FAGX_SensorContact(std::move(Data)));
	}

	return SensorContacts;
}
