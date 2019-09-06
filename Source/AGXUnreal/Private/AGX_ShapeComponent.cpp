// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_ShapeComponent.h"


// Sets default values for this component's properties
UAGX_ShapeComponent::UAGX_ShapeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	UE_LOG(LogTemp, Log, TEXT("ShapeComponent instance crated."));
}


// Called when the game starts
void UAGX_ShapeComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("ShapeComponent ready to simulate"));
}


// Called every frame
void UAGX_ShapeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

