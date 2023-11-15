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
	/**
	 * Set to true if Shovel <-> Terrain contacts should always be removed.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts")
	bool AlwaysRemoveShovelContacts {false};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetAlwaysRemoveShovelContacts(bool InAlwaysRemoveShovelContacts);

	/**
	 * Set if inner shape alone should always create dynamic mass. The alternative is to only create
	 * dynamic mass in the inner shape when primary excavation soil wedges create mass.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableInnerShapeCreateDynamicMass {true};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetEnableInnerShapeCreateDynamicMass(bool InEnableInnerShapeCreateDynamicMass);

	/**
	 * Set whenever the excavation force feedback during PRIMARY excavation should be generated from
	 * particle contacts instead of aggregate contacts.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableParticleForceFeedback {false};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetEnableParticleForceFeedback(bool InSetEnableParticleForceFeedback);

	/**
	 * Set to true if the shovel deformers should make particle free deformations.
	 *
	 * Note, if this is true all excavation modes will make particle free deformations. Even if
	 * enableCreateDynamicMass is set to false for one or more excavation modes.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableParticleFreeDeformers {false};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetEnableParticleFreeDeformers(bool InEnableParticleFreeDeformers);

	/**
	 * Set the minimum submerged cutting edge length fraction (0 to 1) that generates submerged
	 * cutting.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real MinimumSubmergedContactLengthFraction {0.5};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetMinimumSubmergedContactLengthFraction(double InMinimumSubmergedContactLengthFraction);

	/**
	 * Sets the vertical distance under the blade cutting edge that the soil is allowed to instantly
	 * merge up to [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real VerticalBladeSoilMergeDistance {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetVerticalBladeSoilMergeDistance(double InVerticalBladeSoilMergeDistance);

	/**
	 * Set the extension outside the shovel bounding box where soil particle merging is forbidden
	 * [cm].
	 */
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

	/**
	 * The length of each tooth [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothLength {15.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetToothLength(double InToothLength);

	/**
	 * Maximum radius of the shovel teeth [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothMaximumRadius {7.5};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetToothMaximumRadius(double InMaximumToothRadius);

	/**
	 * Minimum radius of the shovel teeth [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothMinimumRadius {1.5};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetToothMinimumRadius(double InMinimumToothRadius);

// Introduced with AGX Dynamics 2.37.
#if 0
	/**
	 * The radius multiplier for extending the inclusion bound with particle radius during
	 * post-excavation with particles in bucket.
	 *
	 * This will only be active post-excavation and NOT during excavation when we have active soil
	 * wedges.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real ParticleInclusionMultiplier {1.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetParticleInclusionMultiplier(double InParticleInclusionMultiplier);
#endif

	/**
	 * Get the vertical penetration depth threshold for when the shovel tooth for penetration
	 * resistance should reach full effectiveness [cm].
	 *
	 * The penetration depth is defined as the vertical distance between the tip of a shovel tooth
	 * and the surface position of the height field. The penetration resistance will increase from a
	 * baseline of 10% until maximum effectiveness is reached when the vertical penetration depth of
	 * the shovel reaches the specified value.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real PenetrationDepthThreshold {50.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetPenetrationDepthThreshold(double InPenetrationDepthThreshold);

	/**
	 * Set the linear scaling coefficient for the penetration force that the terrain will give on
	 * this shovel.
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real PenetrationForceScaling {1.0};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetPenetrationForceScaling(double InPenetrationForceScaling);

	/**
	 * The maximum limit on penetration force that the terrain will generate on the shovel [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real MaximumPenetrationForce {std::numeric_limits<double>::infinity()};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetMaximumPenetrationForce(double InMaximumPenetrationForce);

	/**
	 * The dead-load limit where secondary separation will start to activate where the forward
	 * direction starts to change according to the virtual separation plate created by the material
	 * inside the shovel [?].
	 */
	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real SecondarySeparationDeadloadLimit {0.8};

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	void SetSecondarySeparationDeadloadLimit(double InSecondarySeparationDeadloadLimit);

	/**
	 * Excavation settings for the primary excavation mode, when digging along the cutting
	 * direction in the terrain horizontal plane.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings PrimaryExcavationSettings;

	/**
	 * Excavation settings for the back deformer excavation mode, when digging in the opposite
	 * direction of the cutting direction in the terrain horizontal plane. For example when backside
	 * grading.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformBackExcavationSettings;

	/**
	 * Excavation settings for the right deformer excavation mode, when digging in the clock-wise
	 * direction orthogonal to the cutting direction in the terrain horizontal plane. For example
	 * when pushing or grading.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformRightExcavationSettings;

	/**
	 * Excavation settings for the left deformer excavation mode, when digging in the
	 * counter-clock-wise direction orthogonal to the cutting direction in the terrain horizontal
	 * plane. For example when pushing or grading.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformLeftExcavationSettings;

	// @todo Soil Penetration Model.
	// See
	// https://www.algoryx.se/documentation/complete/agx/html/doc/html/classagxTerrain_1_1Shovel.html#a5686243f6a966d59b17b127a83c3a88a

	/**
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import GUID")
	FGuid ImportGuid;

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.
#endif

	/**
	 * Get the runtime instance for this Shovel Properties.
	 *
	 * If this is a persistent, non-runtime, asset then the corresponding runtime instance is
	 * returned. If no runtime instance has been created yet then a new one is created in the
	 * transient package.
	 *
	 * If this is a runtime instance then self (this) is returned.
	 *
	 * The PlayingWorld parameter must be a pointer to a Game world and if a runtime instance
	 * already exists then it must have been created with the same world.
	 *
	 * @param PlayingWorld The world in which the instance should be created.
	 * @return A Shovel Component instance.
	 */
	UAGX_ShovelProperties* GetOrCreateInstance(UWorld* PlayingWorld);

	/**
	 * Check if this Shovel Properties is a runtime instance or not, i.e. if it is not a persistent
	 * asset.
	 *
	 * @return True if this Shovel Properties is a runtime instance.
	 */
	bool IsInstance() const;

	/**
	 * Get the runtime instance associated with this Shovel Properties.
	 *
	 * Can only ever return non-nullptr for Shovel Properties for which IsAsset returns true.
	 * Can only return non-nullptr while a game session is running.
	 * Will return nullptr if no runtime instance has yet been created.
	 *
	 * @return The runtime instance for this asset.
	 */
	UAGX_ShovelProperties* GetInstance();

	/**
	 * Get the asset that this Shovel Properties runtime instance was created from.
	 *
	 * Can only ever return non-nullptr for Shovel Properties for which IsInstance returns true.
	 *
	 * @return The persistent asset for this runtime instance.
	 */
	UAGX_ShovelProperties* GetAsset();

	/**
	 * Copy all properties from the runtime instance to the persistent asset.
	 *
	 * Can be called on either the asset or the instance, with the same effect.
	 */
	void CommitToAsset();

	/**
	 * Register the given Shovel as one that uses this Shovel Properties runtime instance.
	 *
	 * Changes made to this Shovel Properties will be propagated to the registered Shovels.
	 *
	 * Should not be called on persistent assets.
	 */
	void RegisterShovel(UAGX_ShovelComponent& Shovel);

	/**
	 * Unregister the given Shovel from this Shovel Properties runtime instance.
	 *
	 * The Shovel will no longer be updated when this Shovel Properties is modified.
	 *
	 * Should not be called on persistent assets.
	 */
	void UnregisterShovel(UAGX_ShovelComponent& Shovel);

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

private:
	/// The persistent asset that this runtime instance was created from. Nullptr for assets.
	TWeakObjectPtr<UAGX_ShovelProperties> Asset {nullptr};

	/// The runtime instance that was created from this persistent asset. Nullptr for instances.
	TWeakObjectPtr<UAGX_ShovelProperties> Instance {nullptr}; // Handle multiple worlds?

	/// Registered shovels to be updated when this Shovel Properties is changed. Only populated for
	/// runtime instances, not for persistent assets.
	TArray<TWeakObjectPtr<UAGX_ShovelComponent>> Shovels;
};
