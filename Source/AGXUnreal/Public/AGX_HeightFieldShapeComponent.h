#pragma once

#include "CoreMinimal.h"

#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/HeightFieldShapeBarrier.h"

#include "AGX_HeightFieldShapeComponent.generated.h"

class ALandscape;

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_HeightFieldShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_HeightFieldShapeComponent();

#if 1
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	ALandscape* SourceLandscape;
#else
	UPROPERTY()
	int32 NumVerticesX;

	UPROPERTY()
	int32 NumVerticesY;

	UPROPERTY()
	float SizeX;

	UPROPERTY()
	float SizeY;
#endif

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this HeightField. May return nullptr.
	FHeightFieldShapeBarrier* GetNativeHeightField();

	virtual void UpdateNativeProperties() override;

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;


#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif

private:
	/// Create the AGX Dynamics objects owned by the FBoxShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

private:
	FHeightFieldShapeBarrier NativeBarrier;
};
