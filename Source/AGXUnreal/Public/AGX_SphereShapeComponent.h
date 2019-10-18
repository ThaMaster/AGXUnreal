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

protected:

	void CreateVisualMesh(TArray<FAGX_SimpleMeshTriangle> &Triangles) override;

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const;
#endif

private:
	void CreateNative();

	virtual void ReleaseNative() override;

private:
	FSphereShapeBarrier NativeBarrier;
};
