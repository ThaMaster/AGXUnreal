#include "AGX_AgxDynamicsObjectsAccess.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "SimulationBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "AGX_LogCategory.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>
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
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::GetFrom(const FRigidBodyBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::RigidBody>(Barrier);
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
