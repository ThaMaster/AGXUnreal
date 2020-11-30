#pragma once

// AGXUnreal includes.
#include "Tires/TireBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TireComponent.generated.h"

/**
 * TODO add descr.
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", notplaceable)
class AGXUNREAL_API UAGX_TireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TireComponent();
	virtual ~UAGX_TireComponent() = default;

	UPROPERTY(EditAnywhere, Category = "Rendering")
	bool Visible = true;

	bool HasNative() const;

	FTireBarrier* GetOrCreateNative();

	FTireBarrier* GetNative();

	const FTireBarrier* GetNative() const;

protected:
	TUniquePtr<FTireBarrier> NativeBarrier;

	virtual void BeginPlay() override;

	virtual void AllocateNative() PURE_VIRTUAL(UAGX_TireComponent::CreateNativeImpl, );

	virtual void UpdateNativeProperties()
		PURE_VIRTUAL(UAGX_TireComponent::UpdateNativeProperties, );

private:
	void CreateNative();
};
