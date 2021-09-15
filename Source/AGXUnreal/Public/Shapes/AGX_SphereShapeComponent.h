#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_UpropertyDispatcher.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/SphereShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_SphereShapeComponent.generated.h"

UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_SphereShapeComponent final : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_SphereShapeComponent();

	/// The distance from the center of the sphere to its surface.
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

	FSphereShapeBarrier* GetNativeSphere();

	/**
	 * Copy properties from the given AGX Dynamics sphere into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 *
	 * @param Barrier The AGX Dynamics sphere to copy from.
	 */
	void CopyFrom(const FSphereShapeBarrier& Barrier);

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
	/// Create the AGX Dynamics object owned by this Sphere Shape Component.
	void CreateNative();

private:
	FSphereShapeBarrier NativeBarrier;

#if WITH_EDITOR
	virtual void PostLoad() override;
	void InitPropertyDispatcher();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	FAGX_UpropertyDispatcher<UAGX_SphereShapeComponent> PropertyDispatcher;
#endif
};
