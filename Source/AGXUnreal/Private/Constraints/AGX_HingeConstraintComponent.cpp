#include "Constraints/AGX_HingeConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/HingeBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "AGX_LogCategory.h"

class FRigidBodyBarrier;

UAGX_HingeConstraintComponent::UAGX_HingeConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagTranslational3, EDofFlag::DofFlagRotational1,
		   EDofFlag::DofFlagRotational2},
		  /*bbIsSecondaryConstraintRotational*/ true)
{
}

UAGX_HingeConstraintComponent::~UAGX_HingeConstraintComponent()
{
}

FHingeBarrier* UAGX_HingeConstraintComponent::GetNativeHinge()
{
	const UAGX_HingeConstraintComponent* ConstThis =
		const_cast<const UAGX_HingeConstraintComponent*>(this);
	const FHingeBarrier* ConstHinge = ConstThis->GetNativeHinge();

	// This const_cast is safe because we know that the const version of GetNativeHinge will only
	// return pointers to objects that 'this' owns, and 'this' isn't const so the thing we get a
	// pointer to also isn't const.
	return const_cast<FHingeBarrier*>(ConstHinge);
}

const FHingeBarrier* UAGX_HingeConstraintComponent::GetNativeHinge() const
{
	const FConstraintBarrier* NativeConstraint = GetNative();
	if (NativeConstraint == nullptr)
	{
		return nullptr;
	}

	// We "know" that the ConstraintBarrier can only ever be a HingeBarrier, but it would be nice
	// to be able to make sure. But we can't use dynamic_cast since Unreal Engine disables RTTI.
	const FHingeBarrier* NativeHinge = static_cast<const FHingeBarrier*>(NativeConstraint);
	if (NativeHinge == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Found HingeConstraintComponent with a constraint barrier that isnt' a hinge "
				 "barrier"));
		return nullptr;
	}

	return NativeHinge;
}

void UAGX_HingeConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FHingeBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
