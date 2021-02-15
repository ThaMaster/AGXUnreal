#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/BoxShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BoxShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BoxShapeComponent final : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_BoxShapeComponent();

	/// The distance from the center of the box to its surface along the three cardinal axes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	FVector HalfExtent;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this Box. May return nullptr.
	FBoxShapeBarrier* GetNativeBox();

	virtual void UpdateNativeProperties() override;

	/**
	 * Copy properties from the given AGX Dynamics box into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics box to copy from.
	 */
	void CopyFrom(const FBoxShapeBarrier& Barrier);

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;
#endif

private:
	/// Create the AGX Dynamics objects owned by the FBoxShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

	// BeginPlay/EndPlay is handled by the base class UAGX_ShapeComponent.

private:
	FBoxShapeBarrier NativeBarrier;
};
