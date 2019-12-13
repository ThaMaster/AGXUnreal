#pragma once

#include "Shapes/AGX_ShapeComponent.h"

#include "CoreMinimal.h"

#include "Shapes/HeightFieldShapeBarrier.h"

#include "AGX_HeightFieldShapeComponent.generated.h"

class ALandscape;

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_HeightFieldShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()
public:
	UAGX_HeightFieldShapeComponent();
	virtual ~UAGX_HeightFieldShapeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	ALandscape* SourceLandscape = nullptr;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this HeightField. May return nullptr.
	FHeightFieldShapeBarrier* GetNativeHeightField();

	virtual void UpdateNativeProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent);
#endif

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif

private:
	/// Create the AGX Dynamics objects owned by the FBoxShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

	void OnSourceLandscapeChanged(UObject*, struct FPropertyChangedEvent&);

	void RecenterActorOnLandscape();

	// BeginPlay/EndPlay is handled by the base class UAGX_ShapeComponent.

private:
	FHeightFieldShapeBarrier NativeBarrier;

	FCoreUObjectDelegates::FOnObjectPropertyChanged::FDelegate OnPropertyChangedHandle;
	FDelegateHandle OnPropertyChangedHandleDelegateHandle;
};
