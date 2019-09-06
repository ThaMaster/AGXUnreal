// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_RigidBodyComponent.h"


// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Mass = 10;
	InertiaTensorDiagonal = FVector(1.f, 1.f, 1.f);

	// ...
	UE_LOG(LogTemp, Log, TEXT("RigidBody constructor called."))
}


// Called when the game starts
void UAGX_RigidBodyComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	UE_LOG(LogTemp, Log, TEXT("RigidBody with mass %f ready to simulate."), Mass)

	UE_LOG(LogTemp, Log, TEXT("Searching for geometries."))
}


// Called every frame
void UAGX_RigidBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

