#pragma once

#include "AGX_ShapeComponent.h"

#include "CoreMinimal.h"

#include "SphereShapeBarrier.h"

#include "AGX_SphereShapeComponent.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_SphereShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()
public:
	UAGX_SphereShapeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	FSphereShapeBarrier* GetNativeSphere();

private:
	void CreateNative();

	virtual void ReleaseNative() override;

private:
	FSphereShapeBarrier NativeBarrier;
};
