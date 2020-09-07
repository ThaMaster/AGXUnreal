#include "AGX_AgxDynamicsObjectsAccess.h"

// AGXUnreal includes.
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
	template <typename AgxType, typename BarrierType>
	AgxType* GetFrom(BarrierType* Barrier)
	{
		if (!CheckAgxDynamicsObject<BarrierType>(Barrier))
		{
			return nullptr;
		}

		return Barrier->GetNative()->Native.get();
	}

	template <typename BarrierType>
	bool CheckAgxDynamicsObject(BarrierType* Barrier)
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
}

agx::RigidBody* FAGX_AgxDynamicsObjectsAccess::GetFrom(FRigidBodyBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agx::RigidBody>(Barrier);
}

agxSDK::Simulation* FAGX_AgxDynamicsObjectsAccess::GetFrom(FSimulationBarrier* Barrier)
{
	return AgxDynamicsObjectAccess_Helper::GetFrom<agxSDK::Simulation>(Barrier);
}

agxCollide::Geometry* FAGX_AgxDynamicsObjectsAccess::GetGeometryFrom(FShapeBarrier* Barrier)
{
	if (!AgxDynamicsObjectAccess_Helper::CheckAgxDynamicsObject<FShapeBarrier>(Barrier))
	{
		return nullptr;
	}

	return Barrier->GetNative()->NativeGeometry.get();
}

agxCollide::Shape* FAGX_AgxDynamicsObjectsAccess::GetShapeFrom(FShapeBarrier* Barrier)
{
	if (!AgxDynamicsObjectAccess_Helper::CheckAgxDynamicsObject<FShapeBarrier>(Barrier))
	{
		return nullptr;
	}

	return Barrier->GetNative()->NativeShape.get();
}
