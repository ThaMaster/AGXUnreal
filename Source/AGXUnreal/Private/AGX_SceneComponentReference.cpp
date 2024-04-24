// Copyright 2024, Algoryx Simulation AB.

#include "AGX_SceneComponentReference.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

FAGX_SceneComponentReference::FAGX_SceneComponentReference()
	: FAGX_ComponentReference(USceneComponent::StaticClass())
{
}

USceneComponent* FAGX_SceneComponentReference::GetSceneComponent() const
{
	return Super::GetComponent<USceneComponent>();
}

bool FAGX_SceneComponentReference::SerializeFromMismatchedTag(
	FPropertyTag const& Tag, FStructuredArchive::FSlot Slot)
{
	static const FName ComponentReferenceName("AGX_ComponentReference");
	if (Tag.Type == NAME_StructProperty && Tag.StructName == ComponentReferenceName)
	{
		FAGX_ComponentReference Restored;
		FAGX_ComponentReference::StaticStruct()->SerializeItem(Slot, &Restored, nullptr);
		static_cast<FAGX_ComponentReference&>(*this) = Restored;
		return true;
	}
	else
	{
		return false;
	}
}
