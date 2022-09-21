// Copyright 2022, Algoryx Simulation AB.

#include "AGX_AgxDynamicsObjectsAccess.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "AMOR/MergeSplitPropertiesBarrier.h"
#include "AMOR/MergeSplitThresholdsBarrier.h"
#include "RigidBodyBarrier.h"
#include "SimulationBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Wire/WireBarrier.h"
#include "AGX_LogCategory.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"

#include <agx/RigidBody.h>
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/Hinge.h>
#include <agx/LockJoint.h>
#include <agx/Prismatic.h>

#include <agxSDK/MergeSplitProperties.h>
#include <agxSDK/MergeSplitThresholds.h>
#include <agxSDK/Simulation.h>

#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>

#include <agxTerrain/Terrain.h>

#include <agxWire/Wire.h>

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
	AgxType* GetFromAs(const BarrierType* Barrier)
	{
		if (!CheckAgxDynamicsObject(Barrier))
		{
			return nullptr;
		}

		return Barrier->GetNative()->Native->template as<AgxType>();
	}
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FRigidBodyBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::RigidBody>(Barrier);
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FRigidBodyBarrier& Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::RigidBody>(&Barrier);
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::TryGetFrom(const FRigidBodyBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::TryGetFrom<agx::RigidBody>(Barrier);
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::TryGetFrom(const FRigidBodyBarrier& Barrier)
{
	return AgxDynamicsObjectAccess_Helper::TryGetFrom<agx::RigidBody>(&Barrier);
}

agxSDK::Simulation* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FSimulationBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxSDK::Simulation>(Barrier);
}

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

agxTerrain::Terrain* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FTerrainBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxTerrain::Terrain>(Barrier);
}

agxSDK::MergeSplitProperties* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FMergeSplitPropertiesBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxSDK::MergeSplitProperties>(Barrier);
}

agxSDK::MergeSplitThresholds* FAGX_AgxDynamicsObjectsAccess::GetFrom(
	const FMergeSplitThresholdsBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxSDK::MergeSplitThresholds>(Barrier);
}

agxWire::Wire* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FWireBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxWire::Wire>(Barrier);
}