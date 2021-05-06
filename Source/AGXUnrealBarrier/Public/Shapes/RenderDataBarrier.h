#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// System includes
#include <memory>

struct FRenderDataRef;
struct FAGX_RenderMaterial;

class AGXUNREALBARRIER_API FRenderDataBarrier
{
public:
	FRenderDataBarrier();
	FRenderDataBarrier(FRenderDataBarrier&& Other);
	FRenderDataBarrier(std::unique_ptr<FRenderDataRef>&& InNativeRef);
	~FRenderDataBarrier();

	bool GetShouldRender() const;

	bool HasMesh() const;

	int32 GetNumTriangles() const;
	int32 GetNumIndices() const;

	TArray<uint32> GetIndices() const;
	TArray<FVector> GetPositions() const;
	TArray<FVector> GetNormals() const;
	TArray<FVector2D> GetTextureCoordinates() const;

	bool HasMaterial() const;
	FAGX_RenderMaterial GetMaterial() const;

	FGuid GetGuid() const;

	bool HasNative() const;
	FRenderDataRef* GetNative();
	const FRenderDataRef* GetNative() const;
	void ReleaseNative();

private:
	FRenderDataBarrier(const FRenderDataBarrier& Other) = delete;
	FRenderDataBarrier& operator=(const FRenderDataBarrier& Other) = delete;
	FRenderDataBarrier& operator=(FRenderDataRef&& Other) = delete;

private:
	std::unique_ptr<FRenderDataRef> NativeRef;
};
