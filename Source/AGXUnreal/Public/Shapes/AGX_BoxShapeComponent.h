#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/AGX_AutoFitShapeComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BoxShapeComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BoxShapeComponent final : public UAGX_AutoFitShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_BoxShapeComponent();

	/**
	 * The distance from the center of the box to its surface along the three cardinal axes [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	FVector HalfExtent;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetHalfExtent(const FVector& InHalfExtent);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	FVector GetHalfExtent() const;

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	// ~Begin AGX_AutoFitShapeComponent interface.
	virtual void AutoFit(const TArray<FVector>& Vertices) override;
	// ~End AGX_AutoFitShapeComponent interface.

	/// Get the native AGX Dynamics representation of this Box. May return nullptr.
	FBoxShapeBarrier* GetNativeBox();

	/**
	 * Copy properties from the given AGX Dynamics box into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics box to copy from.
	 */
	void CopyFrom(const FBoxShapeBarrier& Barrier);

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
	/// Create the AGX Dynamics objects owned by this Box Shape Component.
	void CreateNative();

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.

	void InitPropertyDispatcher();
#endif

private:
	FBoxShapeBarrier NativeBarrier;
};
