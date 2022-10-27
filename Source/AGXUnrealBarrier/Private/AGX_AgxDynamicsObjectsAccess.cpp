// Copyright 2022, Algoryx Simulation AB.

#include "AGX_AgxDynamicsObjectsAccess.h"


// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"
#include "AMOR/MergeSplitPropertiesBarrier.h"
#include "AMOR/MergeSplitThresholdsBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Contacts/ContactPointBarrier.h"
#include "Contacts/ContactPointEntity.h"
#include "Contacts/ShapeContactEntity.h"
#include "MassPropertiesBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/TerrainMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "SimulationBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/ShovelBarrier.h"
#include "Tires/TireBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Wire/WireBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/Hinge.h>
#include <agx/LockJoint.h>
#include <agx/MassProperties.h>
#include <agx/Material.h>
#include <agx/Prismatic.h>
#include <agx/RigidBody.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>
#include <agxModel/Tire.h>
#include <agxModel/TwoBodyTire.h>
#include <agxSDK/MergeSplitProperties.h>
#include <agxSDK/MergeSplitThresholds.h>
#include <agxSDK/Simulation.h>
#include <agxTerrain/Terrain.h>
#include <agxTerrain/TerrainMaterial.h>
#include <agxWire/Wire.h>
#include <agxWire/Node.h>
#include "EndAGXIncludes.h"

namespace AgxDynamicsObjectAccess_Helper
{
	template <typename BarrierType>
	bool CheckAgxDynamicsObject(const BarrierType* Barrier)
	{
		if (!Barrier)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Could not get AGX Dynamics native object from barrier. Barrier was nullptr."));
			return false;
		}

		if (!Barrier->HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not get AGX Dynamics native object from barrier since one has not been "
					 "allocated."));
			return false;
		}

		return true;
	}

	template <typename AgxType, typename BarrierType>
	AgxType* GetFrom(const BarrierType* Barrier)
	{
		if (!CheckAgxDynamicsObject<BarrierType>(Barrier))
		{
			return nullptr;
		}

		return Barrier->GetNative()->Native.get();
	}

	template <>
	agxCollide::GeometryContact* GetFrom(const FShapeContactBarrier* Barrier)
	{
		// const_cast safe because AGX Dynamics objects are never declared const as part of a
		// Barrier.
		return const_cast<agxCollide::GeometryContact*>(&Barrier->GetNative()->Native);
	}

	template <>
	agxCollide::ContactPoint* GetFrom(const FContactPointBarrier* Barrier)
	{
		return const_cast<agxCollide::ContactPoint*>(&Barrier->GetNative()->Native);
	}

	template <>
	agx::MassProperties* GetFrom(const FMassPropertiesBarrier* Barrier)
	{
		return Barrier->GetNative()->Native;
	}

	template <typename AgxType, typename BarrierType>
	AgxType* GetFrom(const BarrierType& Barrier)
	{
		return GetFrom<AgxType>(&Barrier);
	}

	template <typename AgxType, typename BarrierType>
	AgxType* TryGetFrom(const BarrierType* Barrier)
	{
		if (Barrier == nullptr || !Barrier->HasNative())
		{
			return nullptr;
		}

		return Barrier->GetNative()->Native.get();
	}

	template <typename AgxType, typename BarrierType>
	AgxType* TryGetFrom(const BarrierType& Barrier)
	{
		return TryGetFrom<AgxType>(&Barrier);
	}

	template <typename AgxType, typename BarrierType>
	AgxType* GetFromAs(const BarrierType* Barrier)
	{
		if (!CheckAgxDynamicsObject(Barrier))
		{
			return nullptr;
		}

		return Barrier->GetNative()->Native->template as<AgxType>();
	}

	// Template specialization for Two Body Tire because our Barrier store a Tire, not a Two Body
	// Tire, since the native is owned by the FTireBarrier base class of FTwoBodyTire. We can't use
	// the regular GetFromAs because Tire inherits from Assembly which inherits virtually from
	// Referenced and static_cast cannot be used with virtual inheritance, which
	// agx::Referenced::as<T>, used by the base function template GetFromAs, 
	template <>
	agxModel::TwoBodyTire* GetFromAs(const FTwoBodyTireBarrier* Barrier)
	{
		if (!CheckAgxDynamicsObject(Barrier))
		{
			return nullptr;
		}

		return dynamic_cast<agxModel::TwoBodyTire*>(Barrier->GetNative()->Native.get());
	}
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FRigidBodyBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::RigidBody>(Barrier);
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FRigidBodyBarrier& Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::RigidBody>(Barrier);
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::TryGetFrom(const FRigidBodyBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::TryGetFrom<agx::RigidBody>(Barrier);
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::TryGetFrom(const FRigidBodyBarrier& Barrier)
{
	return AgxDynamicsObjectAccess_Helper::TryGetFrom<agx::RigidBody>(Barrier);
}

agx::MassProperties* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FMassPropertiesBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::MassProperties>(Barrier);
}

agx::Constraint* GetFrom(const FConstraintBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::Constraint>(Barrier);
}

agx::BallJoint* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FBallJointBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agx::BallJoint>(Barrier);
}

agx::CylindricalJoint* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FCylindricalJointBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agx::CylindricalJoint>(Barrier);
}

agx::DistanceJoint* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FDistanceJointBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agx::DistanceJoint>(Barrier);
}

agx::Hinge* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FHingeBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agx::Hinge>(Barrier);
}

agx::LockJoint* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FLockJointBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agx::LockJoint>(Barrier);
}

agx::Prismatic* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FPrismaticBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agx::Prismatic>(Barrier);
}

agx::Material* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FShapeMaterialBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::Material>(Barrier);
}

agx::ContactMaterial* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FContactMaterialBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::ContactMaterial>(Barrier);
}

// Namespace agxCollide.

agxCollide::Geometry* FAGX_AgxDynamicsObjectsAccess::GetGeometryFrom(const FShapeBarrier* Barrier)
{
	if (!AgxDynamicsObjectAccess_Helper::CheckAgxDynamicsObject<FShapeBarrier>(Barrier))
	{
		return nullptr;
	}

	return Barrier->GetNative()->NativeGeometry.get();
}

agxCollide::Shape* FAGX_AgxDynamicsObjectsAccess::GetShapeFrom(const FShapeBarrier* Barrier)
{
	if (!AgxDynamicsObjectAccess_Helper::CheckAgxDynamicsObject<FShapeBarrier>(Barrier))
	{
		return nullptr;
	}

	return Barrier->GetNative()->NativeShape.get();
}

agxCollide::GeometryContact* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FShapeContactBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxCollide::GeometryContact>(Barrier);
}

agxCollide::ContactPoint* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FContactPointBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxCollide::ContactPoint>(Barrier);
}

// Namespace agxModel.

agxModel::Tire* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FTireBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxModel::Tire>(Barrier);
}

agxModel::TwoBodyTire* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FTwoBodyTireBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFromAs<agxModel::TwoBodyTire>(Barrier);
}

// Namespace agxSDK.

agxSDK::Simulation* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FSimulationBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxSDK::Simulation>(Barrier);
}

agxSDK::MergeSplitProperties* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FMergeSplitPropertiesBarrier* Barrier)
{
	using namespace AgxDynamicsObjectAccess_Helper;
	if (!CheckAgxDynamicsObject(Barrier))
	{
		return nullptr;
	}

	return Barrier->GetNative()->Native;
}

agxSDK::MergeSplitThresholds* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FMergeSplitThresholdsBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxSDK::MergeSplitThresholds>(Barrier);
}

// Namespace agxTerrain.

agxTerrain::Terrain* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FTerrainBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxTerrain::Terrain>(Barrier);
}

agxTerrain::TerrainMaterial* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FTerrainMaterialBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxTerrain::TerrainMaterial>(Barrier);
}

agxTerrain::Shovel* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FShovelBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxTerrain::Shovel>(Barrier);
}

// Namespace agxWire.

agxWire::Wire* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FWireBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxWire::Wire>(Barrier);
}
