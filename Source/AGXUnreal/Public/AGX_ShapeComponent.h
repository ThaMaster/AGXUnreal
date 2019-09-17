#if 0
#pragma once

#include "AGXDynamicsMockup.h"
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_ShapeComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ShapeComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ShapeComponent();

	agx::agxCollide_Geometry* GetNative();
	agx::agxCollide_Geometry* GetOrCreateNative();
	bool HasNative() const;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	void CreateNative();
	virtual void CreateNativeShapes(TArray<agx::agxCollide_ShapeRef>& OutNativeShapes);

private:
	agx::agxCollide_GeometryRef NativeGeometry;
	TArray<agx::agxCollide_ShapeRef> NativeShapes;
};
#endif
