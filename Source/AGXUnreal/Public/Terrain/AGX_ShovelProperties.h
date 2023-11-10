// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/AGX_ShovelExcavationSettings.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_ShovelProperties.generated.h"

class UAGX_ShovelComponent;
class UWorld;

/**
 * An asset used to hold configuration properties for Shovel Components.
 */
UCLASS(ClassGroup = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_ShovelProperties : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Contacts")
	bool AlwaysRemoveShovelContacts {false};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetAlwaysRemoveShovelContacts(bool InAlwaysRemoveShovelContacts);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableInnerShapeCreateDynamicMass {true};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetEnableInnerShapeCreateDynamicMass(bool InEnableInnerShapeCreateDynamicMass);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableParticleForceFeedback {false};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetEnableParticleForceFeedback (bool InSetEnableParticleForceFeedback);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableParticleFreeDeformers {false};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetEnableParticleFreeDeformers(bool InEnableParticleFreeDeformers);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real MinimumSubmergedContactLengthFraction {0.5};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetMinimumSubmergedContactLengthFraction(double InMinimumSubmergedContactLengthFraction);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real VerticalBladeSoilMergeDistance {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetVerticalBladeSoilMergeDistance(double InVerticalBladeSoilMergeDistance);

	UPROPERTY(EditAnywhere, Category = "Shovel Properties")
	FAGX_Real NoMergeExtensionDistance {50.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetNoMergeExtensionDistance(double InNoMergeExtensionDistance);

	/**
	 * Number of teeth of the Shovel.
	 */
	UPROPERTY(EditAnywhere, Category = "Teeth")
	int32 NumberOfTeeth {6};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetNumberOfTeeth(int32 InNumberOfTeeth);

	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothLength {15.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetToothLength(double InToothLength);

	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothMaximumRadius {7.5};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetToothMaximumRadius(double InMaximumToothRadius);

	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothMinimumRadius {1.5};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetToothMinimumRadius(double InMinimumToothRadius);

// Introduced with AGX Dynamics 2.37.
#if 0
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real ParticleInclusionMultiplier {1.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetParticleInclusionMultiplier(double InParticleInclusionMultiplier);
#endif

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real PenetrationDepthThreshold {50.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetPenetrationDepthThreshold(double InPenetrationDepthThreshold);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real PenetrationForceScaling {1.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetPenetrationForceScaling(double InPenetrationForceScaling);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real MaximumPenetrationForce {std::numeric_limits<double>::infinity()};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetMaximumPenetrationForce(double InMaximumPenetrationForce);

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real SecondarySeparationDeadloadLimit {0.8};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetSecondarySeparationDeadloadLimit(double InSecondarySeparationDeadloadLimit);

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings PrimaryExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformBackExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformRightExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformLeftExcavationSettings;

	// @todo Soil Penetration Model.
	// See
	// https://www.algoryx.se/documentation/complete/agx/html/doc/html/classagxTerrain_1_1Shovel.html#a5686243f6a966d59b17b127a83c3a88a

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.
	#endif

	UAGX_ShovelProperties* GetOrCreateInstance(UWorld* PlayingWorld);

	bool IsInstance() const;
	UAGX_ShovelProperties* GetInstance();
	UAGX_ShovelProperties* GetAsset();

	void CommitToAsset();

	void RegisterShovel(UAGX_ShovelComponent& Shovel);
	void UnregisterShovel(UAGX_ShovelComponent& Shovel);

private:
#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every UPROPERTY in PostEditChangeProperty and PostEditChangeChainProperty.
	void InitPropertyDispatcher();
#endif

private:
	TWeakObjectPtr<UAGX_ShovelProperties> Asset {nullptr};
	TWeakObjectPtr<UAGX_ShovelProperties> Instance {nullptr}; // Handle multiple worlds?
	TArray<TWeakObjectPtr<UAGX_ShovelComponent>> Shovels; // Only populated for instances, not for assets.
};
