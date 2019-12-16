#pragma once

#include "Shapes/AGX_ShapeComponent.h"
#include "CoreMinimal.h"

#include "Shapes/CylinderShapeBarrier.h"

#include "AGX_CylinderShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CylinderShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_CylinderShapeComponent();

	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	double Height;

	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	double Radius;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this Cylinder. May return nullptr.
	FCylinderShapeBarrier* GetNativeCylinder();

	virtual void UpdateNativeProperties() override;

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;
#endif

private:
	/// Create the AGX Dynamics objects owned by the FCylinderShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

	// BeginPlay/EndPlay is handled by the base class UAGX_ShapeComponent.

private:
	FCylinderShapeBarrier NativeBarrier;
};
