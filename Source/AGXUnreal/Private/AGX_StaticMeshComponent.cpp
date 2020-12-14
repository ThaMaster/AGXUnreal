#include "AGX_StaticMeshComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/AGX_ShapeMaterialBase.h"
#include "Materials/AGX_ShapeMaterialInstance.h"

// Unreal Engien includes.
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"

UAGX_StaticMeshComponent::UAGX_StaticMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UAGX_StaticMeshComponent::PostLoad()
{
	/// \todo Determine if this is necessary. Alternative implementations include the virtual
	/// OnMeshChanged member function, the virtual OnCreatePhysicsState, and
	/// GetStaticMesh()->OnMeshChanged.
	OnStaticMeshChanged().AddUObject(this, &UAGX_StaticMeshComponent::UpdateCollisionShapes);
}

bool UAGX_StaticMeshComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FRigidBodyBarrier* UAGX_StaticMeshComponent::GetNative()
{
	return &NativeBarrier;
}

const FRigidBodyBarrier* UAGX_StaticMeshComponent::GetNative() const
{
	return &NativeBarrier;
}

FRigidBodyBarrier* UAGX_StaticMeshComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		AllocateNative();
	}
	return GetNative();
}

void UAGX_StaticMeshComponent::OnMeshChanged()
{
	UE_LOG(LogAGX, Warning, TEXT("Calling RefreshCollisionShapes from OnMeshChanged."))
	RefreshCollisionShapes();
}

void UAGX_StaticMeshComponent::UpdateCollisionShapes(UStaticMeshComponent* Self)
{
	if (Self != this)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Self != this.  That is surprising. What should we do in this case?"));
		return;
	}

	UE_LOG(LogAGX, Warning, TEXT("Calling RefreshCollisionShapes from UpdateCollisionShapes."))
	RefreshCollisionShapes();
}

void UAGX_StaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		AllocateNative();
	}
}

void UAGX_StaticMeshComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
	{
		GetNative()->ReleaseNative();
	}
}

void UAGX_StaticMeshComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ReadTransformFromNative();
}

namespace AGX_StaticMeshComponent_helpers
{
// I would like to use RefreshCollisionShapes(PhysicsShapes, CollisionShapes) to
// reorder PhysicsShapes so that they match the new ordering of CollisionShapes.
// To do this I need a way to identify which of the new CollisionShapes each
// PhysicsShape belong to.
#if 0
	template <typename FCollisionShape>
	void RefreshCollisionShapes(
		TArray<FAGX_Shape>& PhysicsShapes, TArray<FCollisionShape>& CollisionShapes)
	{

	}
#endif
}

bool UAGX_StaticMeshComponent::ShouldCreatePhysicsState() const
{
	/// \note I'm not entirely sure on the consequences of doing this. I want to maintain my own
	/// physics state, which is the AGX Dynamics state. I do not want the PhysX code to start doing
	/// stuff because of this. And it's not even the actual AGX Dynamics state but the local state
	/// we keep in order to create the AGX Dynamics objects later, on BeginPlay.
	///
	/// All I want is to keep the TArray<FAGX_Shape> containers in sync with the collision shapes
	/// stored in the StaticMesh asset.
	return true;
}

void UAGX_StaticMeshComponent::OnCreatePhysicsState()
{
	UE_LOG(LogAGX, Warning, TEXT("Calling RefreshCollisionShapes from OnCreatePhysicsState."))
	RefreshCollisionShapes();
	bPhysicsStateCreated = true;
}

void UAGX_StaticMeshComponent::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	UE_LOG(LogAGX, Warning, TEXT("PropertyChangedEvent called"));

	if (Event.GetPropertyName() == GetMemberNameChecked_StaticMesh())
	{
		UE_LOG(LogAGX, Warning, TEXT("Calling RefreshCollisionShapes from PostEditChangeProperty."))
		RefreshCollisionShapes();
	}
}

void UAGX_StaticMeshComponent::PreEditChange(FProperty* Property)
{
	if (Property->GetName() == GetMemberNameChecked_StaticMesh().ToString())
	{
		if (GetStaticMesh() != nullptr)
		{
			GetStaticMesh()->OnMeshChanged.Remove(MeshChangedHandle);
			MeshChangedHandle.Reset();
		}
	}
}

void UAGX_StaticMeshComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	Super::PostEditChangeChainProperty(Event);
	UE_LOG(
		LogAGX, Warning, TEXT("ChainPropertyChangedEvent for %s."),
		*Event.GetPropertyName().ToString());

	/// \todo Only do this if the changed property actually affects the shapes.
	/// Is there any such property?
	///
	/// The comment above needs some clarification. I want to detect when the collision shapes
	/// within the StaticMesh asset is changed. I'm not sure this will work since the mesh pointer
	/// is the same, it's the thing pointed to that is modified. We need a callback when
	/// PostEditChange(Chain)?Property is called on the StaticMesh asset. There is `OnMeshChanged`,
	/// I will experiment with that, but I think that is for vertices only.
	/// There is also RecreatePhysicsState which StaticMeshEditor calls from time to time, but it
	if (Event.GetPropertyName() == GetMemberNameChecked_StaticMesh())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Calling RefreshCollisionShapes from PostEditChangeChainProperty."))
		RefreshCollisionShapes();
		if (GetStaticMesh() != nullptr)
		{
			MeshChangedHandle = GetStaticMesh()->OnMeshChanged.AddUObject(
				this, &UAGX_StaticMeshComponent::OnMeshChanged);
		}
	}
}

namespace AGX_StaticMeshComponent_helpers
{
	/**
	 * Swap all shape material assets for their material instance version.
	 * @param Shape The shape for which the material should be swapped.
	 * @param World The world in which material instances should be created.
	 */
	void SwapInMaterialInstance(FAGX_Shape& Shape, UWorld& World)
	{
		// No asset pointers should remain after a swap. If we fail to create an instance then we
		// should fall back to the default material, with a warning.
		UAGX_ShapeMaterialBase* Asset = Shape.Material;
		Shape.Material = nullptr;

		if (Asset == nullptr)
		{
			// If the asset material is nullptr then the instance material should also be nullptr,
			// AGX Dynamics will interpret that to mean the default material.
			return;
		}

		if (!World.IsGameWorld())
		{
			// Only create instances in game worlds, never in the editor world.
			return;
		}

		FShapeMaterialBarrier* Barrier = nullptr;
		UAGX_ShapeMaterialInstance* Instance =
			Cast<UAGX_ShapeMaterialInstance>(Asset->GetOrCreateInstance(&World));
		if (Instance == nullptr)
		{
			/// \todo Better error message.
			UE_LOG(LogAGX, Error, TEXT("Could not create a ShapeMaterialInstance."));
			return;
		}

		Shape.Material = Instance;
	}

	FAGX_Shape& GetShape(TArray<FAGX_Shape>& Shapes, int32 Index, FAGX_Shape& Default)
	{
		return Shapes.IsValidIndex(Index) ? Shapes[Index] : Default;
	}

	FVector GetHalfExtent(const FKBoxElem& Box)
	{
		return FVector(Box.X / 2.0f, Box.Y / 2.0f, Box.Z / 2.0f);
	}
}

void UAGX_StaticMeshComponent::AllocateNative()
{
	using namespace AGX_StaticMeshComponent_helpers;

	if (GetWorld() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot create AGX Dynamics representation of AGX_StaticMeshComponent '%s' "
				 "because no world is available."), *GetName());
		return;
	}
	UWorld& World = *GetWorld();

	/// \todo Replace with early-out once we're confident that things work the way they should.
	check(!NativeBarrier.HasNative());
	check(SphereBarriers.Num() == 0);
	check(BoxBarriers.Num() == 0);

	UE_LOG(LogAGX, Warning, TEXT("Calling RefreshCollisionShapes from AllocateNative."))
	RefreshCollisionShapes();
	NativeBarrier.AllocateNative();

	SwapInMaterialInstance(DefaultShape, World);
	for (auto& Sphere : Spheres)
	{
		SwapInMaterialInstance(Sphere, World);
	}
	for (auto& Box : Boxes)
	{
		SwapInMaterialInstance(Box, World);
	}

	if (GetStaticMesh() != nullptr)
	{
		FKAggregateGeom& CollisionShapes = GetStaticMesh()->BodySetup->AggGeom;

		// Copy sphere data from the collision spheres to the barrier spheres.
		TArray<FKSphereElem>& CollisionSpheres = CollisionShapes.SphereElems;
		SphereBarriers.Reserve(CollisionSpheres.Num());
		for (int32 I = 0; I < CollisionSpheres.Num(); ++I)
		{
			FKSphereElem& Collision = CollisionSpheres[I];
			FAGX_Shape& Shape = GetShape(Spheres, I, DefaultShape);
			FSphereShapeBarrier Barrier;
			Barrier.AllocateNative();
			check(Barrier.HasNative());
			Barrier.SetRadius(Collision.Radius);
			Barrier.SetLocalPosition(Collision.Center);
			Barrier.SetLocalRotation(FQuat::Identity); // Rotation does not matter for spheres.
			Barrier.SetEnableCollisions(Shape.bCanCollide);
			if (Shape.Material != nullptr)
			{
				FShapeMaterialBarrier* MaterialBarrier =
					Shape.Material->GetOrCreateShapeMaterialNative(&World);
				if (MaterialBarrier != nullptr)
				{
					Barrier.SetMaterial(*MaterialBarrier);
				}
			}
			Barrier.SetName(Collision.GetName().ToString());
			Barrier.AddCollisionGroups(Shape.CollisionGroups);
			NativeBarrier.AddShape(&Barrier);
			SphereBarriers.Add(std::move(Barrier));
		}

		// Copy box data from the collision boxes to the barrier boxes.
		TArray<FKBoxElem>& CollisionBoxes = CollisionShapes.BoxElems;
		BoxBarriers.Reserve(CollisionBoxes.Num());
		for (int32 I = 0; I < CollisionBoxes.Num(); ++I)
		{
			FKBoxElem& Collision = CollisionBoxes[I];
			FAGX_Shape& Shape = GetShape(Boxes, I, DefaultShape);
			FBoxShapeBarrier Barrier;
			Barrier.AllocateNative();
			check(Barrier.HasNative());
			Barrier.SetHalfExtents(GetHalfExtent(Collision));
			Barrier.SetLocalPosition(Collision.Center);
			Barrier.SetLocalRotation(Collision.Rotation.Quaternion());
			Barrier.SetEnableCollisions(Shape.bCanCollide);
			if (Shape.Material != nullptr)
			{
				FShapeMaterialBarrier* MaterialBarrier =
					Shape.Material->GetOrCreateShapeMaterialNative(&World);
				if (MaterialBarrier != nullptr)
				{
					Barrier.SetMaterial(*MaterialBarrier);
				}
			}
			Barrier.SetName(Collision.GetName().ToString());
			Barrier.AddCollisionGroups(Shape.CollisionGroups);
			NativeBarrier.AddShape(&Barrier);
			BoxBarriers.Add(std::move(Barrier));
		}
	}

	WriteTransformToNative();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->AddRigidBody(this);
}

namespace AGX_StaticMeshComponent_helpers
{
	void ResizeShapeArray(TArray<FAGX_Shape>& Shapes, FAGX_Shape& DefaultShape, int32 Num)
	{
		int32 OldNum = Shapes.Num();
		Shapes.SetNum(Num);
		for (int32 I = OldNum; I < Num; ++I)
		{
			Shapes[I] = DefaultShape;
		}
	}
}

/*
 Call order when assigning a StaticMesh with 0 spheres and 3 boxes to a new SimulationObject:

 // This is the virtual function inherited from StaticMeshComponent.
 Calling RefreshCollisionShapes from OnCreatePhysicsState.
 Have 0 spheres and 0 boxes.

 // This is the delegate callback from StaticMeshComponent.
 UpdateCollisionShapes called.
 Calling RefreshCollisionShapes from UpdateCollisionShapes.
 Have 0 spheres and 3 boxes.

 // This is because we changed a property on SimulationObject.
 PropertyChangedEvent called
 Calling RefreshCollisionShapes from PostEditChangeProperty.
 Have 0 spheres and 3 boxes.

 // This is also because we changed a property on SimulationObject.
 ChainPropertyChangedEvent for StaticMesh.
 Calling RefreshCollisionShapes from PostEditChangeChainProperty.
 Have 0 spheres and 3 boxes.


 Call order when adding a collision sphere with the SimulationObject Details Panel open and visible.

 // This is the virtual function inherited from StaticMeshComponent.
 Calling RefreshCollisionShapes from OnCreatePhysicsState.
 Have 0 spheres and 3 boxes.

 The same call, OnCreatePhysicsState, is also made when deleting a collision shape.
 */
void UAGX_StaticMeshComponent::RefreshCollisionShapes()
{
	using namespace AGX_StaticMeshComponent_helpers;

	if (GetStaticMesh() == nullptr)
	{
		Spheres.SetNum(0);
		Boxes.SetNum(0);
		return;
	}

	FKAggregateGeom& CollisionShapes = GetStaticMesh()->BodySetup->AggGeom;
	TArray<FKSphereElem>& CollisionSpheres = CollisionShapes.SphereElems;
	TArray<FKBoxElem>& CollisionBoxes = CollisionShapes.BoxElems;

	UE_LOG(
		LogAGX, Warning, TEXT("Want %d spheres and %d boxes."), CollisionSpheres.Num(),
		CollisionBoxes.Num());

	UE_LOG(LogAGX, Warning, TEXT("Had %d spheres and %d boxes."), Spheres.Num(), Boxes.Num());
	ResizeShapeArray(Spheres, DefaultShape, CollisionSpheres.Num());
	ResizeShapeArray(Boxes, DefaultShape, CollisionBoxes.Num());
	UE_LOG(LogAGX, Warning, TEXT("Have %d spheres and %d boxes."), Spheres.Num(), Boxes.Num());
}

void UAGX_StaticMeshComponent::ReadTransformFromNative()
{
	check(HasNative());
	const FVector NewLocation = NativeBarrier.GetPosition();
	const FQuat NewRotation = NativeBarrier.GetRotation();

	/// \todo Consider supporting other transformation targets, such as parent and root.
	/// Should we? If so, why?
	const FVector OldLocation = GetComponentLocation();
	const FVector LocationDelta = NewLocation - OldLocation;
	MoveComponent(LocationDelta, NewRotation, false);
}

void UAGX_StaticMeshComponent::WriteTransformToNative()
{
	check(HasNative());
	NativeBarrier.SetPosition(GetComponentLocation());
	NativeBarrier.SetRotation(GetComponentQuat());
}
