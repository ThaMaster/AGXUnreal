#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireRenderIteratorBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireRenderIterator.generated.h"

struct FAGX_WireNode;

USTRUCT(BlueprintType, Category = "AGX Dynamics")
struct AGXUNREAL_API FAGX_WireRenderIterator
{
	GENERATED_BODY();

public:
	FAGX_WireRenderIterator() = default;
	FAGX_WireRenderIterator(const FAGX_WireRenderIterator& InOther);
	FAGX_WireRenderIterator(FWireRenderIteratorBarrier&& InBarrier);
	FAGX_WireRenderIterator& operator=(const FAGX_WireRenderIterator& InOther);

	bool operator==(const FAGX_WireRenderIterator& InOther) const;
	bool operator!=(const FAGX_WireRenderIterator& InOther) const;

	FAGX_WireNode Get() const;
	FAGX_WireRenderIterator& Inc();
	FAGX_WireRenderIterator& Dec();
	FAGX_WireRenderIterator Next() const;
	FAGX_WireRenderIterator Prev() const;

private:
	FWireRenderIteratorBarrier Barrier;
};
