#include "AGX_Stepper.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"

#include "Engine/GameInstance.h"

AAGX_Stepper::AAGX_Stepper()
{
	/// \todo When should we tick things? Here we set the TickGroup for the
	///       stepper to be PrePhysics while everything else retain the default
	///       DuringPhysics. This means that the other classes are guaranteed
	///       to see the new state for this tick.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	UE_LOG(LogAGX, Log, TEXT("Stepper created"));
}

AAGX_Stepper::~AAGX_Stepper()
{
	UE_LOG(LogAGX, Log, TEXT("Stepper destroyed."));
}

void AAGX_Stepper::Tick(float DeltaTime)
{
#if 0
	UE_LOG(LogAGX, Log, TEXT("Stepper ticking"));
#endif
	Super::Tick(DeltaTime);
	UGameInstance* Game = GetGameInstance();
	UAGX_Simulation* Simulation = Game->GetSubsystem<UAGX_Simulation>();
	Simulation->Step(DeltaTime);
}
