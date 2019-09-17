#if 0

// Fill out your copyright notice in the Description page of Project Settings.

#include "AGX_ShapeComponent.h"

#include "AGX_LogCategory.h"

// Sets default values for this component's properties
UAGX_ShapeComponent::UAGX_ShapeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	UE_LOG(LogAGX, Log, TEXT("ShapeComponent instance crated."));
}

agx::agxCollide_Geometry* UAGX_ShapeComponent::GetNative()
{
	return NativeGeometry;
}

agx::agxCollide_Geometry* UAGX_ShapeComponent::GetOrCreateNative()
{
	if (NativeGeometry != nullptr)
	{
		return NativeGeometry;
	}

	CreateNative();
	check(NativeGeometry != nullptr);
	return NativeGeometry;
}

bool UAGX_ShapeComponent::HasNative() const
{
	return NativeGeometry != nullptr;
}

void UAGX_ShapeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAGX_ShapeComponent::CreateNative()
{
	NativeGeometry = agx::allocate(TEXT("agxCollide::Geometry"));
	CreateNativeShapes(NativeShapes);
	for (agx::agxCollide_ShapeRef& Shape : NativeShapes)
	{
		agx::call(TEXT("agxCollide::add(Shape);"));
	}
}

void UAGX_ShapeComponent::CreateNativeShapes(TArray<agx::agxCollide_ShapeRef>& /*OutNativeShapes*/)
{
	unimplemented();
}

// Called when the game starts
void UAGX_ShapeComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogAGX, Log, TEXT("ShapeComponent ready to simulate"));
}
#endif
