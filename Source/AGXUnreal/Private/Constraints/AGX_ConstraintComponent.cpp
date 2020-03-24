#include "Constraints/AGX_ConstraintComponent.h"

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintConstants.h"
#include "UObject/UObjectGlobals.h"

/// \todo Determine which of these are really needed.
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/AGX_ConstraintDofGraphicsComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintIconGraphicsComponent.h"
#include "Constraints/ConstraintBarrier.h"

EDofFlag ConvertDofsArrayToBitmask(const TArray<EDofFlag>& LockedDofsOrdered)
{
	uint8 bitmask(0);
	for (EDofFlag flag : LockedDofsOrdered)
	{
		bitmask |= (uint8) flag;
	}
	return (EDofFlag) bitmask;
}

TMap<EGenericDofIndex, int32> BuildNativeDofIndexMap(const TArray<EDofFlag>& LockedDofsOrdered)
{
	TMap<EGenericDofIndex, int32> DofIndexMap;
	for (int32 NativeIndex = 0; NativeIndex < LockedDofsOrdered.Num(); ++NativeIndex)
	{
		switch (LockedDofsOrdered[NativeIndex])
		{
			case EDofFlag::DOF_FLAG_TRANSLATIONAL_1:
				DofIndexMap.Add(EGenericDofIndex::TRANSLATIONAL_1, NativeIndex);
				break;
			case EDofFlag::DOF_FLAG_TRANSLATIONAL_2:
				DofIndexMap.Add(EGenericDofIndex::TRANSLATIONAL_2, NativeIndex);
				break;
			case EDofFlag::DOF_FLAG_TRANSLATIONAL_3:
				DofIndexMap.Add(EGenericDofIndex::TRANSLATIONAL_3, NativeIndex);
				break;
			case EDofFlag::DOF_FLAG_ROTATIONAL_1:
				DofIndexMap.Add(EGenericDofIndex::ROTATIONAL_1, NativeIndex);
				break;
			case EDofFlag::DOF_FLAG_ROTATIONAL_2:
				DofIndexMap.Add(EGenericDofIndex::ROTATIONAL_2, NativeIndex);
				break;
			case EDofFlag::DOF_FLAG_ROTATIONAL_3:
				DofIndexMap.Add(EGenericDofIndex::ROTATIONAL_3, NativeIndex);
				break;
			default:
				checkNoEntry();
		}
	}

	// General mappings:
	DofIndexMap.Add(EGenericDofIndex::ALL_DOF, -1);

	return DofIndexMap;
}

UAGX_ConstraintComponent::UAGX_ConstraintComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAGX_ConstraintComponent::UAGX_ConstraintComponent(const TArray<EDofFlag>& LockedDofsOrdered)
	: bEnable(true)
	, SolveType(EAGX_SolveType::ST_DIRECT)
	, Elasticity(
		  ConstraintConstants::DefaultElasticity(), ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, Damping(ConstraintConstants::DefaultDamping(), ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, ForceRange(
		  ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax(),
		  ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, LockedDofsBitmask(ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, LockedDofs(LockedDofsOrdered)
	, NativeDofIndexMap(BuildNativeDofIndexMap(LockedDofsOrdered))
{
	BodyAttachment1.FrameDefiningActor = GetOwner();
	BodyAttachment2.FrameDefiningActor = GetOwner();

#if WITH_EDITOR
	BodyAttachment1.OnFrameDefiningActorChanged(this);
	BodyAttachment2.OnFrameDefiningActorChanged(this);
#endif

	// Create UAGX_ConstraintDofGraphicsComponent as child component.
	{
		DofGraphicsComponent1 = CreateDefaultSubobject<UAGX_ConstraintDofGraphicsComponent>(
			TEXT("DofGraphicsComponent1"));

		DofGraphicsComponent1->Constraint = this;
		DofGraphicsComponent1->SetupAttachment(this);
		DofGraphicsComponent1->bHiddenInGame = true;
		DofGraphicsComponent1->AttachmentId = 1;
	}
	{
		DofGraphicsComponent2 = CreateDefaultSubobject<UAGX_ConstraintDofGraphicsComponent>(
				TEXT("DofGraphicsComponent2"));

		DofGraphicsComponent2->Constraint = this;
		DofGraphicsComponent2->SetupAttachment(this);
		DofGraphicsComponent2->bHiddenInGame = true;
		DofGraphicsComponent2->AttachmentId = 2;
	}

	// Create UAGX_ConstraintIconGraphicsComponent as child component.
	{
		IconGraphicsComponent = CreateDefaultSubobject<UAGX_ConstraintIconGraphicsComponent>(
			TEXT("IconGraphicsComponent"));

		IconGraphicsComponent->Constraint = this;
		IconGraphicsComponent->SetupAttachment(this);
		IconGraphicsComponent->bHiddenInGame = true;
	}
}

UAGX_ConstraintComponent::~UAGX_ConstraintComponent()
{
#if WITH_EDITOR
	BodyAttachment1.OnDestroy(this);
	BodyAttachment2.OnDestroy(this);
#endif
}

FConstraintBarrier* UAGX_ConstraintComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

FConstraintBarrier* UAGX_ConstraintComponent::GetNative()
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

bool UAGX_ConstraintComponent::HasNative() const
{
	return NativeBarrier.Get() != nullptr && NativeBarrier->HasNative();
}

bool UAGX_ConstraintComponent::AreFramesInViolatedState(float Tolerance, FString* OutMessage) const
{
	if (BodyAttachment1.GetRigidBody() == nullptr || BodyAttachment2.GetRigidBody() == nullptr)
	{
		/// \todo Check if it is possible to create a single-body constraint, i.e., body
		/// constrained to the world, where the constraint is in an initially violated state. That
		/// is, can we create a world attachment frame that does not align with the local attachment
		/// frame of the body.
		return false;
	}

	auto WriteMessage = [OutMessage](EDofFlag Dof, float Error) {
		if (OutMessage == nullptr)
		{
			return;
		}

		UEnum* DofEnum = FindObject<UEnum>(nullptr, TEXT("EDofFlag"));
		FString DofString = DofEnum->GetValueAsString(Dof);
		*OutMessage += FString::Printf(TEXT("%s has violation %f."), *DofString, Error);
	};

	FVector Location1 = BodyAttachment1.GetGlobalFrameLocation();
	FQuat Rotation1 = BodyAttachment1.GetGlobalFrameRotation();

	FVector Location2 = BodyAttachment2.GetGlobalFrameLocation();
	FQuat Rotation2 = BodyAttachment2.GetGlobalFrameRotation();

	FVector Location2InLocal1 = Rotation1.Inverse().RotateVector(Location2 - Location1);
	FQuat Rotation2InLocal1 = Rotation1.Inverse() * Rotation2;

	if (IsDofLocked(EDofFlag::DOF_FLAG_TRANSLATIONAL_1))
	{
		if (FMath::Abs(Location2InLocal1.X) > Tolerance)
		{
			WriteMessage(EDofFlag::DOF_FLAG_TRANSLATIONAL_1, FMath::Abs(Location2InLocal1.X));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DOF_FLAG_TRANSLATIONAL_2))
	{
		if (FMath::Abs(Location2InLocal1.Y) > Tolerance)
		{
			WriteMessage(EDofFlag::DOF_FLAG_TRANSLATIONAL_2, FMath::Abs(Location2InLocal1.Y));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DOF_FLAG_TRANSLATIONAL_3))
	{
		if (FMath::Abs(Location2InLocal1.Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DOF_FLAG_TRANSLATIONAL_3, FMath::Abs(Location2InLocal1.Z));
			return true;
		}
	}

	/// \todo Checks below might not be correct for ALL scenarios. What if there is for example a 90
	/// degrees rotation around one axis and then a rotation around another?

	if (IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_1))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisY().Z) > Tolerance)
		{
			WriteMessage(
				EDofFlag::DOF_FLAG_ROTATIONAL_1, FMath::Abs(Rotation2InLocal1.GetAxisY().Z));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_2))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisX().Z) > Tolerance)
		{
			WriteMessage(
				EDofFlag::DOF_FLAG_ROTATIONAL_2, FMath::Abs(Rotation2InLocal1.GetAxisX().Z));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_3))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisX().Y) > Tolerance)
		{
			WriteMessage(
				EDofFlag::DOF_FLAG_ROTATIONAL_3, FMath::Abs(Rotation2InLocal1.GetAxisX().Y));
			return true;
		}
	}
	return false;
}

EDofFlag UAGX_ConstraintComponent::GetLockedDofsBitmask() const
{
	return LockedDofsBitmask;
}

bool UAGX_ConstraintComponent::IsDofLocked(EDofFlag Dof) const
{
	return static_cast<uint8>(LockedDofsBitmask) & static_cast<uint8>(Dof);
}

#if WITH_EDITOR
void UAGX_ConstraintComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// The leaf property that was changed. May be nested in a struct.
	FName PropertyName = (PropertyChangedEvent.Property != NULL)
							 ? PropertyChangedEvent.Property->GetFName()
							 : NAME_None;

	// The root property that contains the property that was changed.
	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL)
								   ? PropertyChangedEvent.MemberProperty->GetFName()
								   : NAME_None;

	if (MemberPropertyName == PropertyName)
	{
		// Property of this class changed.
	}
	else
	{
		// Property of an aggregate struct changed.

		FAGX_ConstraintBodyAttachment* ModifiedBodyAttachment =
			(MemberPropertyName ==
			 GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, BodyAttachment1))
				? &BodyAttachment1
				: ((MemberPropertyName ==
					GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, BodyAttachment2))
					   ? &BodyAttachment2
					   : nullptr);

		if (ModifiedBodyAttachment)
		{
			if (PropertyName ==
				GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, FrameDefiningActor))
			{
				// TODO: Code below needs to be triggered also when modified through code!
				// Editor-only probably OK though, since it is just for Editor convenience.
				// See FCoreUObjectDelegates::OnObjectPropertyChanged.
				// Or/additional add Refresh button to AAGX_ConstraintFrameActor's Details Panel
				// that rebuilds the constraint usage list.
				ModifiedBodyAttachment->OnFrameDefiningActorChanged(this);
			}

			/// \todo This is a bit of a hack and it may be possible to remove it.
			/// The intention was to handle the Blueprint editor case, where it's not possible to
			/// select the Actor that will be created when the Blueprint is instantiated as the
			/// OwningActor in the RigidBodyReference. Here we set the Constraint's owner as the
			/// OwningActor, meaning that the RigidBodyReference will reference something in the
			/// "local scope". This should be the Blueprint itself, or a representation thereof,
			/// but it's unclear to me if it will work like that.
			if (ModifiedBodyAttachment->RigidBody.OwningActor == nullptr)
			{
				ModifiedBodyAttachment->RigidBody.FallbackOwningActor = GetOwner();
				if (ModifiedBodyAttachment->RigidBody.FallbackOwningActor == nullptr)
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("BodyAttachment in '%s' got nullptr fallback owning actor."),
						*GetName());
				}
			}
		}
	}

	UE_LOG(
		LogAGX, Log, TEXT("PostEditChangeProperty: PropertyName = %s, MemberPropertyName = %s"),
		*PropertyName.ToString(), *MemberPropertyName.ToString());
}
#endif

#define TRY_SET_DOF_VALUE(SourceStruct, GenericDof, Func)         \
	{                                                             \
		int32 Dof;                                                \
		if (ToNativeDof(GenericDof, Dof) && 0 <= Dof && Dof <= 5) \
			Func(SourceStruct[(int32) GenericDof], Dof);          \
	}

#define TRY_SET_DOF_RANGE_VALUE(SourceStruct, GenericDof, Func)                                    \
	{                                                                                              \
		int32 Dof;                                                                                 \
		if (ToNativeDof(GenericDof, Dof) && 0 <= Dof && Dof <= 5)                                  \
			Func(SourceStruct[(int32) GenericDof].Min, SourceStruct[(int32) GenericDof].Max, Dof); \
	}

void UAGX_ConstraintComponent::UpdateNativeProperties()
{
	if (HasNative())
	{
		NativeBarrier->SetEnable(bEnable);
		NativeBarrier->SetSolveType(SolveType);

		// TODO: Could just loop NativeDofIndexMap instead!!

		TRY_SET_DOF_VALUE(
			Elasticity, EGenericDofIndex::TRANSLATIONAL_1, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(
			Elasticity, EGenericDofIndex::TRANSLATIONAL_2, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(
			Elasticity, EGenericDofIndex::TRANSLATIONAL_3, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::ROTATIONAL_1, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::ROTATIONAL_2, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::ROTATIONAL_3, NativeBarrier->SetElasticity);

		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::TRANSLATIONAL_1, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::TRANSLATIONAL_2, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::TRANSLATIONAL_3, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::ROTATIONAL_1, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::ROTATIONAL_2, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::ROTATIONAL_3, NativeBarrier->SetDamping);

		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::TRANSLATIONAL_1, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::TRANSLATIONAL_2, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::TRANSLATIONAL_3, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::ROTATIONAL_1, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::ROTATIONAL_2, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::ROTATIONAL_3, NativeBarrier->SetForceRange);
	}
}

#undef TRY_SET_DOF_VAlUE
#undef TRY_SET_DOF_RANGE

#if WITH_EDITOR
void UAGX_ConstraintComponent::PostLoad()
{
	Super::PostLoad();
	BodyAttachment1.OnFrameDefiningActorChanged(this);
	BodyAttachment2.OnFrameDefiningActorChanged(this);

	// Provide a default owning actor, the owner of this component, if no owner has been specified
	// for the RigidBodyReferences. This is always the case when the constraint has been created
	// as part of an Actor Blueprint.
	for (FAGX_RigidBodyReference* BodyReference :
		 {&BodyAttachment1.RigidBody, &BodyAttachment2.RigidBody})
	{
		BodyReference->FallbackOwningActor = nullptr;
		if (BodyReference->OwningActor == nullptr)
		{
			BodyReference->OwningActor = GetOwner();
			BodyReference->CacheCurrentRigidBody();
		}
	}
}

void UAGX_ConstraintComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	BodyAttachment1.OnFrameDefiningActorChanged(this);
	BodyAttachment2.OnFrameDefiningActorChanged(this);
}

void UAGX_ConstraintComponent::BeginDestroy()
{
	Super::BeginDestroy();
	BodyAttachment1.OnDestroy(this);
	BodyAttachment2.OnDestroy(this);
}
#endif

void UAGX_ConstraintComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		CreateNative();
	}
}

bool UAGX_ConstraintComponent::ToNativeDof(EGenericDofIndex GenericDof, int32& NativeDof)
{
	if (const int32* NativeDofPtr = NativeDofIndexMap.Find(GenericDof))
	{
		NativeDof = *NativeDofPtr;
		return true;
	}
	else
	{
		return false;
	}
}

void UAGX_ConstraintComponent::CreateNative()
{
	/// \todo Verify that we are in-game!

	check(!HasNative());

	CreateNativeImpl();

	if (HasNative())
	{
		UpdateNativeProperties();
		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
		Simulation->GetNative()->AddConstraint(NativeBarrier.Get());
	}
	else
	{
		UE_LOG(
			LogAGX, Error, TEXT("Constraint %s in %s: Unable to create constraint."),
			*GetFName().ToString(), *GetOwner()->GetName());
	}
}
