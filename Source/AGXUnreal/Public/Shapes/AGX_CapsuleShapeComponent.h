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

	/// The distance from the center of any the two half-spheres at each end to their surface.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Radius;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this Capsule. May return nullptr.
	FCapsuleShapeBarrier* GetNativeCapsule();

	virtual void UpdateNativeProperties() override;

	/**
	 * Copy properties from the given AGX Dynamics Capsule into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics Capsule to copy from.
	 */
	void CopyFrom(const FCapsuleShapeBarrier& Barrier);

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;

	// ~Begin UAGX_ShapeComponent interface.
	virtual FShapeBarrier* GetNativeBarrier() override;
	// ~End UAGX_ShapeComponent interface.

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;
#endif

private:
	/// Create the AGX Dynamics objects owned by the FCapsuleShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

	// BeginPlay/EndPlay is handled by the base class UAGX_ShapeComponent.

private:
	FCapsuleShapeBarrier NativeBarrier;
};
