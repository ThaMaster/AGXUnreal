#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_UpropertyDispatcher.h"
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

	/// The distance from the the surface of one of its end disks to the other.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Height;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetHeight(float InHeight);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetHeight() const;

	/// The distance from the center of the cylinder to the cylindrical surface.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Radius;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetRadius(float InRadius);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetRadius() const;

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	/// Get the native AGX Dynamics representation of this Cylinder. May return nullptr.
	FCylinderShapeBarrier* GetNativeCylinder();


	/**
	 * Copy properties from the given AGX Dynamics cylinder into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics cylinder to copy from.
	 */
	void CopyFrom(const FCylinderShapeBarrier& Barrier);

protected:

	// ~Begin UAGX_ShapeComponent interface.
	virtual FShapeBarrier* GetNativeBarrier() override;
	virtual const FShapeBarrier* GetNativeBarrier() const override;
	virtual void ReleaseNative() override;
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;
#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;
#endif
	// ~End UAGX_ShapeComponent interface.

private:
	/// Create the AGX Dynamics object owned by this Cylinder Shape Component.
	void CreateNative();

	#if WITH_EDITOR
	virtual void PostLoad() override;
	void InitPropertyDispatcher();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	FAGX_UpropertyDispatcher<UAGX_CylinderShapeComponent> PropertyDispatcher;
#endif

	FCylinderShapeBarrier NativeBarrier;
};
