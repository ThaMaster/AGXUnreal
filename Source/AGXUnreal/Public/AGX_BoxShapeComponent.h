#pragma once

#include "AGX_ShapeComponent.h"
#include "CoreMinimal.h"

#include "BoxShapeBarrier.h"

#include "AGX_BoxShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BoxShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_BoxShapeComponent();

	/// The distance from the center of the box to it's surface along the three cardinal axes.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector HalfExtent;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this Box. May return nullptr.
	FBoxShapeBarrier* GetNativeBox();

private:
	/// Create the AGX Dynamics objects owned by the FBoxShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

	// BeginPlay/EndPlay is handled by the base class UAGX_ShapeComponent.

private:
	FBoxShapeBarrier NativeBarrier;
};
