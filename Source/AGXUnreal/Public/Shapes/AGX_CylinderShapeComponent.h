#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/CylinderShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CylinderShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CylinderShapeComponent final : public UAGX_ShapeComponent
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

	/**
	 * Copy properties from the given AGX Dynamics cylinder into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics cylinder to copy from.
	 */
	void CopyFrom(const FCylinderShapeBarrier& Barrier);

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
