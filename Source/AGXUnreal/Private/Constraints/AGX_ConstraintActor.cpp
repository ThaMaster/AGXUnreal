#include "Constraints/AGX_ConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintComponent.h"

AAGX_ConstraintActor::AAGX_ConstraintActor()
{
	// A default constructed AAGX_Constraint doesn't really make sense. Only
	// used by Unreal Engine's behind-the-scenes objects. Consider marking this
	// class Abstract in the UCLASS specifiers.
}

AAGX_ConstraintActor::AAGX_ConstraintActor(UAGX_ConstraintComponent* InConstraintComponent)
{
	ConstraintComponent = InConstraintComponent;
	ConstraintComponent->SetFlags(ConstraintComponent->GetFlags() | RF_Transactional);
	ConstraintComponent->SetMobility(EComponentMobility::Movable);
#if WITH_EDITORONLY_DATA
	// Disable the root SceneComponent's while blob.
	ConstraintComponent->bVisualizeComponent = false;
#endif
	SetRootComponent(ConstraintComponent);
}

AAGX_ConstraintActor::~AAGX_ConstraintActor()
{
}

UAGX_ConstraintComponent* AAGX_ConstraintActor::GetConstraintComponent()
{
	return ConstraintComponent;
}

const UAGX_ConstraintComponent* AAGX_ConstraintActor::GetConstraintComponent() const
{
	return ConstraintComponent;
}

bool AAGX_ConstraintActor::IsLevelBoundsRelevant() const
{
	return false;
}
