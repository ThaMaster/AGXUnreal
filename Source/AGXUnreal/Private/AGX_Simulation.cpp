#include "AGX_Simulation.h"

#include "AGX_LogCategory.h"

void UAGX_Simulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: new agxSDK::Simulation"));
	NativeBarrier.AllocateNative();
}


void UAGX_Simulation::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: delete agxSDK::Simulation"));
	NativeBarrier.ReleaseNative();
}
