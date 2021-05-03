#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireRenderIteratorBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireRenderIterator.generated.h"

struct FAGX_WireNode;

/**
 * An iterator that iterates through the renderable nodes in a Wire Component. Instances are created
 * by the Wire Component's GetRenderBeginIterator and GetRenderEndIterator member functions.
 */
USTRUCT(BlueprintType, Category = "AGX Dynamics")
struct AGXUNREAL_API FAGX_WireRenderIterator
{
	GENERATED_BODY();

public:
	FAGX_WireRenderIterator() = default;
	FAGX_WireRenderIterator(const FAGX_WireRenderIterator& InOther);
	FAGX_WireRenderIterator(FWireRenderIteratorBarrier&& InBarrier);
	FAGX_WireRenderIterator& operator=(const FAGX_WireRenderIterator& InOther);

	bool HasNative() const;

	bool operator==(const FAGX_WireRenderIterator& InOther) const;
	bool operator!=(const FAGX_WireRenderIterator& InOther) const;

	/**
	 * Get the Wire Node currently pointed to by this iterator. Must not be called when this
	 * iterator compares equal to GetRenderEndIterator or when HasNative returns false. Iterators
	 * fetched from an UAGX_WireComponent for which HasRenderNodes returns true i guaranteed to have
	 * a native, and all iterators produced by Inc, Dec, Next, and Prev on such iterators will also
	 * be guaranteed to have a native until an iterator comparing equal to the return value of
	 * GetRenderEndIterator is reached.
	 *
	 * @return The Wire Node currently pointed to by this iterator.
	 */
	FAGX_WireNode Get() const;

	/// Move to the next Wire Node.
	FAGX_WireRenderIterator& Inc();

	/// Move to the previous Wire Node.
	FAGX_WireRenderIterator& Dec();

	/// Create an iterator to the next Wire Node.
	FAGX_WireRenderIterator Next() const;

	// Create an iterator to the previous Wire Node.
	FAGX_WireRenderIterator Prev() const;

private:
	FWireRenderIteratorBarrier Barrier;
};
