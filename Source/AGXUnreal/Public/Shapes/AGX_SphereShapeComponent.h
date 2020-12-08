#pragma once

// AGX Dynamics for Unreal includes.
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	float Radius;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	FSphereShapeBarrier* GetNativeSphere();

	virtual void UpdateNativeProperties() override;

	/**
	 * Copy properties from the given AGX Dynamics sphere into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics sphere to copy from.
	 */
	void CopyFrom(const FSphereShapeBarrier& Barrier);

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;
#endif

private:
	void CreateNative();

	virtual void ReleaseNative() override;

private:
	FSphereShapeBarrier NativeBarrier;
};
