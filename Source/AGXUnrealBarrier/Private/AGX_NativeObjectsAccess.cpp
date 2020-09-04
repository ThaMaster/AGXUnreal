#include "AGX_NativeObjectsAccess.h"

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

agx::RigidBody* FAGX_NativeObjectsAccess::BarrierToNative(FRigidBodyBarrier* Barrier)
{
	if (!Barrier)
	{
		UE_LOG(LogAGX, Error, TEXT("Could not get AGX native from barrier. Barrier was nullptr."));
		return nullptr;
	}

	if (!Barrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not get AGX native from barrier. Native object has not been "
				 "allocated."));
		return nullptr;
	}

	return Barrier->GetNative()->Native.get();
}

agxSDK::Simulation* FAGX_NativeObjectsAccess::BarrierToNative(FSimulationBarrier* Barrier)
{
	if (!Barrier)
	{
		UE_LOG(LogAGX, Error, TEXT("Could not get AGX native from barrier. Barrier was nullptr."));
		return nullptr;
	}

	if (!Barrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not get AGX native from barrier. Native object has not been "
				 "allocated."));
		return nullptr;
	}

	return Barrier->GetNative()->Native.get();
}

agxCollide::Geometry* FAGX_NativeObjectsAccess::BarrierToNativeGeometry(FShapeBarrier* Barrier)
{
	if (!Barrier)
	{
		UE_LOG(LogAGX, Error, TEXT("Could not get AGX native from barrier. Barrier was nullptr."));
		return nullptr;
	}

	if (!Barrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not get AGX native from barrier. Native object has not been "
				 "allocated."));
		return nullptr;
	}

	return Barrier->GetNative()->NativeGeometry.get();
}

agxCollide::Shape* FAGX_NativeObjectsAccess::BarrierToNativeShape(FShapeBarrier* Barrier)
{
	if (!Barrier)
	{
		UE_LOG(LogAGX, Error, TEXT("Could not get AGX native from barrier. Barrier was nullptr."));
		return nullptr;
	}

	if (!Barrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not get AGX native from barrier. Native object has not been "
				 "allocated."));
		return nullptr;
	}

	return Barrier->GetNative()->NativeShape.get();
}
