#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/CapsuleShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CapsuleShapeComponent.generated.h"

UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CapsuleShapeComponent final : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_CapsuleShapeComponent();

	/// The distance from the centers of the capsule's half-spheres at each end.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Height;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetHeight(float InHeight);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetHeight() const;

	/// The distance from the center of any the two half-spheres at each end to their surface.
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

	/// Get the native AGX Dynamics representation of this Capsule. May return nullptr.
	FCapsuleShapeBarrier* GetNativeCapsule();

	/**
	 * Copy properties from the given AGX Dynamics Capsule into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics Capsule to copy from.
	 */
	void CopyFrom(const FCapsuleShapeBarrier& Barrier);

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
	/// Create the AGX Dynamics object owned by this Capsule Shape Component.
	void CreateNative();

private:
#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	// ~End UObject interface.

	void InitPropertyDispatcher();
#endif

	FCapsuleShapeBarrier NativeBarrier;
};
