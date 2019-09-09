#pragma once

#include "AGX_ShapeComponent.h"
#include "CoreMinimal.h"


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


	/// Get the native AGX Dynamics representation of this Box. Create it if necessary.
	agx::agxCollide_Box* GetOrCreateBox();

	/// Get the native AGX Dynamics representation of this Box. May return nullptr.
	agx::agxCollide_Box* GetBox();

private:
	/// Called when the AGX Dynamics object is to be created.
	virtual void CreateNativeShapes(TArray<agx::agxCollide_ShapeRef>& OutNativeShapes) override;

	void CreateNativeBox();

private:
	agx::agxCollide_BoxRef Native;
};
