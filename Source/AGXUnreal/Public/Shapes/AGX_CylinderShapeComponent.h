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

	/**
	 * Set to true to enable the Pulley property on this cylinder.
	 *
	 * When enabled contact points with wires will only be created on the center line of the
	 * cylinder perimeter, preventing the wire from slipping off the cylinder.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Wire", Meta = (EditCondition = "!bGypsy"))
	bool bPulley = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetPulley(bool bInPulley);

	/**
	 * Set to true to enable the Gypsy property on this cylinder.
	 *
	 * When enabled contact points with wires will only be created on the center line of the
	 * cylinder perimeter, preventing the wire from slipping off of the Cylinder.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Wire", Meta = (EditCondition = "!bPulley"))
	bool bGypsy = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetGypsy(bool bInGypsy);

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
		const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif
	// ~End UAGX_ShapeComponent interface.

private:
	/// Create the AGX Dynamics object owned by this Cylinder Shape Component.
	void CreateNative();

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	// ~End UObject interface.

	void InitPropertyDispatcher();
#endif

	FCylinderShapeBarrier NativeBarrier;
};
