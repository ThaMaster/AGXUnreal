#include "AGX_Simulation.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Stepper.h"
#include "AGX_LogCategory.h"

#include "Engine/World.h"

void UAGX_Simulation::AddRigidBody(UAGX_RigidBodyComponent* body)
{
	NativeBarrier.AddRigidBody(body->GetNative());
}

void UAGX_Simulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: new agxSDK::Simulation"));
	NativeBarrier.AllocateNative();
	GetWorld()->SpawnActor(AAGX_Stepper::StaticClass());
	/// \todo Instead of creating an Actor for Step triggerng, one may use
	///       FTickableObjectBase or FTickFunction. It's not clear to me how to
	///       use these other classes.
}

void UAGX_Simulation::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: delete agxSDK::Simulation"));
	NativeBarrier.ReleaseNative();
}

void UAGX_Simulation::Step(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	while (DeltaTime >= TimeStep)
	{
		UE_LOG(LogAGX, Log, TEXT("AGX_CALL: agxSDK::Simulation::stepForward"));
		NativeBarrier.Step();
		DeltaTime -= TimeStep;
	}
	LeftoverTime = DeltaTime;
}
