// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_Simulation.h"

#include "AGX_LogCategory.h"


void UAGX_Simulation::Initialize(FSubsystemCollectionBase& Collection) /*override*/
{
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: new agxSDK::Simulation"));
}


void UAGX_Simulation::Deinitialize() /*override*/
{
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: delete agxSDK::Simulation"));
}