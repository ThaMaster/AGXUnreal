#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "Materials/ContactMaterialBarrier.h" /// \todo Shouldn't be necessary here!
#include "AGX_ContactMaterialInstance.generated.h"

class FContactMaterialBarrier;
class UAGX_ContactMaterialAsset;
class UAGX_MaterialBase;

/**
 * Represents a native AGX Contact Material in-game. Should never exist when not playing.
 *
 * Should only be created using the static function CreateFromAsset, which copyies data from its
 * sibling class UAGX_ContactMaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_ContactMaterialInstance : public UAGX_ContactMaterialBase
{
	GENERATED_BODY()

public:
	static UAGX_ContactMaterialInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ContactMaterialAsset* Source);

public:
	virtual ~UAGX_ContactMaterialInstance();

	virtual UAGX_ContactMaterialAsset* GetAsset() override;

	FContactMaterialBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	FContactMaterialBarrier* GetNative();

	bool HasNative() const;

	void UpdateNativeProperties();

	// ~Begin UAGX_ContactMaterialBase interface.
	virtual void SetContactSolver(EAGX_ContactSolver InContactSolver) override;
	virtual void SetContactReductionMode(EAGX_ContactReductionMode InReductionMode) override;
	virtual void SetContactReductionBinResolution(uint8 InBinResolution) override;
	virtual void SetUseContactAreaApproach(bool bInUseContactAreaApproach) override;
	virtual void SetMinElasticRestLength(float InMinLength) override;
	virtual void SetMaxElasticRestLength(float InMaxLength) override;
	virtual void SetFrictionModel(EAGX_FrictionModel InFrictionModel) override;
	virtual void SetSurfaceFrictionEnabled(bool bInSurfaceFrictionEnabled) override;
	virtual void SetFrictionCoefficient(float InFrictionCoefficient) override;
	virtual void SetSecondaryFrictionCoefficient(float InSecondaryFrictionCoefficient) override;
	virtual void SetUseSecondaryFrictionCoefficient(bool bInUseSecondaryFrictionCoefficient) override;
	virtual void SetSurfaceViscosity(float InSurfaceViscosity) override;
	virtual void SetSecondarySurfaceViscosity(float InSecondarySurfaceViscosity) override;
	virtual void SetUseSecondarySurfaceViscosity(bool bInUserSecondarySurfaceViscosity) override;
	virtual void SetRestitution(float Restitution) override;
	virtual void SetDamping(float Damping) override;
	virtual void SetYoungsModulus(float YoungsModulus) override;
	virtual void SetAdhesiveForce(float AdhesiveForce) override;
	virtual void SetAdhesiveOverlap(float AdhesiveOverlap) override;
	virtual UAGX_ContactMaterialInstance* GetInstance() override;
	virtual UAGX_ContactMaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
	// ~End UAGX_ContactMaterialBase interface.

private:
	// Creates the native AGX Contact Material and adds it to the simulation.
	void CreateNative(UWorld* PlayingWorld);

	/// \todo This member is probably not necessary.. Remove it?
	TWeakObjectPtr<UAGX_ContactMaterialAsset> SourceAsset;

	TUniquePtr<FContactMaterialBarrier> NativeBarrier;
};
