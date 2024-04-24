// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_SceneComponentReference.generated.h"

class AActor;

/**
 * A reference to a USceneComponent.
 *
 * See comment on FAGX_ComponentReference for usage instructions and limitations.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_SceneComponentReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_SceneComponentReference();

	USceneComponent* GetSceneComponent() const;

	/// Called by Unreal Engine when de-serializing an FAGX_SceneComponentReference but some other
	/// type was found in the archive. FAGX_ComponentReference is read but all other types are
	/// rejected.
	bool SerializeFromMismatchedTag(struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot);
};

/**
 * A struct that informs Unreal Engine about how FAGX_SceneComponentReference can be used.
 */
template <>
struct TStructOpsTypeTraits<FAGX_SceneComponentReference>
	: public TStructOpsTypeTraitsBase2<FAGX_SceneComponentReference>
{
	// clang-format off
	enum
	{
		// This is the subset of flags from TSTructOpsTypeTraits that we care about for FAGX_SceneComponentReference.

		// Tell Unreal Engine that while restoring an FAGX_SceneComponentReference it is OK to find
		// a FAGX_ComponentReference since we can convert to a FAGX_SceneComponentReference.
		WithStructuredSerializeFromMismatchedTag = true, // struct has an FStructuredArchive-based SerializeFromMismatchedTag function for converting from other property tags.
	};
	// clang-format on
};

UCLASS()
class AGXUNREAL_API UAGX_SceneComponentReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Scene Component Reference")
	static USceneComponent* GetSceneComponent(UPARAM(Ref) FAGX_SceneComponentReference& Reference)
	{
		return Reference.GetSceneComponent();
	}
};
