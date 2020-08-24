#include "AGX_NativeObjectsAccess.h"

// AGXUnreal includes.
#include "AGXRefs.h "
#include "RigidBodyBarrier.h"
#include "SimulationBarrier.h"
#include "AGX_LogCategory.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agxSDK/Simulation.h>
#include "EndAGXIncludes.h"

agx::RigidBody* FAGX_NativeObjectsAccess::BarrierToNative(FRigidBodyBarrier* Barrier)
{
	if (!Barrier)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not get AGX native from barrier. Barrier is was nullptr."));
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

	return Barrier->GetNative()->Native;
}

agxSDK::Simulation* FAGX_NativeObjectsAccess::BarrierToNative(FSimulationBarrier* Barrier)
{
	if (!Barrier)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not get AGX native from barrier. Barrier is was nullptr."));
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

	return Barrier->GetNative()->Native;
}
