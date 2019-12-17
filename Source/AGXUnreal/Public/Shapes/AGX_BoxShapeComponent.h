#pragma once

#include "Shapes/AGX_ShapeComponent.h"
#include "CoreMinimal.h"

#include "Shapes/BoxShapeBarrier.h"

#include "AGX_BoxShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BoxShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_BoxShapeComponent();

	/// The distance from the center of the box to it's surface along the three cardinal axes.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	FVector HalfExtent;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this Box. May return nullptr.
	FBoxShapeBarrier* GetNativeBox();

	virtual void UpdateNativeProperties() override;

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
