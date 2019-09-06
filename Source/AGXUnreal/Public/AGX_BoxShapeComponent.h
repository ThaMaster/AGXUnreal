#pragma once

#include "AGX_ShapeComponent.h"
#include "CoreMinimal.h"

#include "AGXDynamicsMockup.h"

#include "AGX_BoxShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BoxShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_BoxShapeComponent();

	/// The distance from the center of the box to it's surface along the three cardinal axes.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector HalfExtent;

	/// Get the native AGX Dynamics representation of this Box. Create it if necessary.
	agx::agxCollide_Box* getOrCreateNative();

	/// Get the native AGX Dynamics representation of this Box. May return nullptr;
	agx::agxCollide_Box* getNative();

	/// Return true if the AGX Dynamics object has been created. False otherwise.
	bool HasNative() const;

protected:
	// Called when the game starts.
	virtual void BeginPlay() override;

private:
	/// Called when the AGX Dynamics object is to be created.
	void InitializeNative();

private:
	agx::agxCollide_BoxRef Native = nullptr;
	agx::agxCollide_GeometryRef NativeGeometry = nullptr;
};
