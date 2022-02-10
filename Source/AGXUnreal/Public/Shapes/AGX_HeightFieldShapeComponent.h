// Copyright 2022, Algoryx Simulation AB.


#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/HeightFieldShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_HeightFieldShapeComponent.generated.h"

class ALandscape;

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_HeightFieldShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_HeightFieldShapeComponent();
	virtual ~UAGX_HeightFieldShapeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	ALandscape* SourceLandscape = nullptr;

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	/// Get the native AGX Dynamics representation of this HeightField. May return nullptr.
	FHeightFieldShapeBarrier* GetNativeHeightField();

	/**
	 * Copy properties from the given AGX Dynamics Height Field into this component.
	 * Will only copy properties inherited from UAGX_ShapeComponent, no changes to the Landscape
	 * will be performed.
	 *
	 * @param Barrier The AGX Dynamics Height Field to copy from.
	 */
	void CopyFrom(const FHeightFieldShapeBarrier& Barrier);

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.

protected:

	// ~Begin UAGX_ShapeComponent interface.
	virtual FShapeBarrier* GetNativeBarrier() override;
	virtual const FShapeBarrier* GetNativeBarrier() const override;
	virtual void ReleaseNative() override;
	virtual void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;
	virtual void UpdateNativeGlobalTransform() override;
#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif
	// ~End UAGX_ShapeComponent interface.

private:
	/// Create the AGX Dynamics objects owned by the FBoxShapeBarrier.
	void CreateNative();

	void OnSourceLandscapeChanged(UObject*, struct FPropertyChangedEvent&);

	void RecenterActorOnLandscape();

private:
	FHeightFieldShapeBarrier NativeBarrier;

#if WITH_EDITORONLY_DATA
	FCoreUObjectDelegates::FOnObjectPropertyChanged::FDelegate OnPropertyChangedHandle;
	FDelegateHandle OnPropertyChangedHandleDelegateHandle;
#endif
};
