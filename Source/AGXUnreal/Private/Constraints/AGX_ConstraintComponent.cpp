#include "Constraints/AGX_ConstraintComponent.h"

// AGX Dynamics for Unreal includes.
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
			case EDofFlag::DofFlagTranslational1:
				DofIndexMap.Add(EGenericDofIndex::Translational1, NativeIndex);
				break;
			case EDofFlag::DofFlagTranslational2:
				DofIndexMap.Add(EGenericDofIndex::Translational2, NativeIndex);
				break;
			case EDofFlag::DofFlagTranslational3:
				DofIndexMap.Add(EGenericDofIndex::Translational3, NativeIndex);
				break;
			case EDofFlag::DofFlagRotational1:
				DofIndexMap.Add(EGenericDofIndex::Rotational1, NativeIndex);
				break;
			case EDofFlag::DofFlagRotational2:
				DofIndexMap.Add(EGenericDofIndex::Rotational2, NativeIndex);
				break;
			case EDofFlag::DofFlagRotational3:
				DofIndexMap.Add(EGenericDofIndex::Rotational3, NativeIndex);
				break;
			default:
				checkNoEntry();
		}
	}

	// General mappings:
	DofIndexMap.Add(EGenericDofIndex::AllDof, -1);

	return DofIndexMap;
}

UAGX_ConstraintComponent::UAGX_ConstraintComponent()
	: BodyAttachment1(this)
	, BodyAttachment2(this)
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAGX_ConstraintComponent::UAGX_ConstraintComponent(const TArray<EDofFlag>& LockedDofsOrdered)
	: BodyAttachment1(this)
	, BodyAttachment2(this)
	, bEnable(true)
	, SolveType(EAGX_SolveType::StDirect)
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
	// Using an AGX_ConstraintComponent in a blueprint instance has in some cases caused crashes
	// during project startup. It seems to happen for one of the 'behind-the-scenes' created objects
	// that are created for blueprints, typically with name postfix '_GEN_VARIABLE'. This
	// 'behind-the-scenes' created object crashes for some reason when trying to attach a
	// DofGraphicsComponent or IconGraphicsComponent to it. Therefore we detect when this
	// constructor is run for such an object by checking if its parent is null (which will only be
	// the case for such an object) and in that case we do not attach the GraphicsComponents to it.
	if (GetOwner() != nullptr)
	{
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
}

UAGX_ConstraintComponent::~UAGX_ConstraintComponent()
{
#if WITH_EDITOR
	BodyAttachment1.OnDestroy(this);
	BodyAttachment2.OnDestroy(this);
#endif
}

void UAGX_ConstraintComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after InitProperties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	//
	// The intention is to provide by default a local scope that is the Actor outer that this
	// Component is part of. If the OwningActor is set anywhere else, such as in the Details Panel,
	// then that "else" should overwrite the value set here shortly.
	//
	// We use GetTypedOuter because we worry that in some cases the Owner may not yet have been set
	// but there will always be an outer chain. This worry may be unfounded.
	BodyAttachment1.RigidBody.OwningActor = GetTypedOuter<AActor>();
	BodyAttachment2.RigidBody.OwningActor = GetTypedOuter<AActor>();
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

const FConstraintBarrier* UAGX_ConstraintComponent::GetNative() const
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

	if (IsDofLocked(EDofFlag::DofFlagTranslational1))
	{
		if (FMath::Abs(Location2InLocal1.X) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagTranslational1, FMath::Abs(Location2InLocal1.X));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagTranslational2))
	{
		if (FMath::Abs(Location2InLocal1.Y) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagTranslational2, FMath::Abs(Location2InLocal1.Y));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagTranslational3))
	{
		if (FMath::Abs(Location2InLocal1.Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagTranslational3, FMath::Abs(Location2InLocal1.Z));
			return true;
		}
	}

	/// \todo Checks below might not be correct for ALL scenarios. What if there is for example a 90
	/// degrees rotation around one axis and then a rotation around another?

	if (IsDofLocked(EDofFlag::DofFlagRotational1))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisY().Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagRotational1, FMath::Abs(Rotation2InLocal1.GetAxisY().Z));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagRotational2))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisX().Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagRotational2, FMath::Abs(Rotation2InLocal1.GetAxisX().Z));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagRotational3))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisX().Y) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagRotational3, FMath::Abs(Rotation2InLocal1.GetAxisX().Y));
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

namespace
{
	FAGX_ConstraintBodyAttachment* SelectByName(
		const FName& Name, FAGX_ConstraintBodyAttachment* Attachment1,
		FAGX_ConstraintBodyAttachment* Attachment2)
	{
		if (Name == GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, BodyAttachment1))
		{
			return Attachment1;
		}
		else if (Name == GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, BodyAttachment2))
		{
			return Attachment2;
		}
		else
		{
			return nullptr;
		}
	}
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
			SelectByName(MemberPropertyName, &BodyAttachment1, &BodyAttachment2);

		if (ModifiedBodyAttachment)
		{
			// TODO: Code below needs to be triggered also when modified through code!
			// Editor-only probably OK though, since it is just for Editor convenience.
			// See FCoreUObjectDelegates::OnObjectPropertyChanged.
			// Or/additional add Refresh button to AAGX_ConstraintFrameActor's Details Panel
			// that rebuilds the constraint usage list.

			/// \todo This property is three levels deep instead of two for the FrameDefiningActor.
			/// The middle layer is an FAGX_SceneComponentReference. Here we don't know if the
			/// property change happened in the ModifiedBodyAttachment's RigidBody or its
			/// FrameDefiningComponent. Assuming we have to update.
			/// Consider doing this in PostEditChangeChainProperty instead.
			if (PropertyName == GET_MEMBER_NAME_CHECKED(FAGX_SceneComponentReference, OwningActor))
			{
				ModifiedBodyAttachment->OnFrameDefiningComponentChanged(this);
			}
			if (PropertyName ==
				GET_MEMBER_NAME_CHECKED(FAGX_SceneComponentReference, SceneComponentName))
			{
				ModifiedBodyAttachment->OnFrameDefiningComponentChanged(this);
			}

			// We must always have an OwningActor. The user clearing/setting OwningActor to
			// None/nullptr really means "search the local scope", which we achieve by setting
			// OwningActor to the closest AActor outer.
			//
			// This does not work when we are in the Blueprint Editor. In this case we don't have
			// an AActor outer at all, and no Owner. The outer chain contains something like the
			// following:
			// - Hinge_GEN_VARIABLE of type AGX_HingeConstraintComponent
			// - BP_Blueprint_C of type BlueprintGeneratedClass
			// - /Game/BP_Blueprint of type Package
			//
			// The interesting piece here is BlueprintGeneratedClass, from which it may be possible
			// to get a list of names of RigidBodyComponents. Possibly via the
			// SimpleConstructionScript member and then GetAllNodes. Experimentation needed.
			if (ModifiedBodyAttachment->RigidBody.OwningActor == nullptr)
			{
				ModifiedBodyAttachment->RigidBody.OwningActor = GetTypedOuter<AActor>();
			}

			if (ModifiedBodyAttachment->FrameDefiningSource == EAGX_FrameDefiningSource::Other &&
				ModifiedBodyAttachment->FrameDefiningComponent.OwningActor == nullptr)
			{
				ModifiedBodyAttachment->FrameDefiningComponent.FallbackOwningActor = GetOwner();
			}
		}
	}
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
			Elasticity, EGenericDofIndex::Translational1, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(
			Elasticity, EGenericDofIndex::Translational2, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(
			Elasticity, EGenericDofIndex::Translational3, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::Rotational1, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::Rotational2, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::Rotational3, NativeBarrier->SetElasticity);

		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::Translational1, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::Translational2, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::Translational3, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::Rotational1, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::Rotational2, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::Rotational3, NativeBarrier->SetDamping);

		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::Translational1, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::Translational2, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::Translational3, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::Rotational1, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::Rotational2, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(
			ForceRange, EGenericDofIndex::Rotational3, NativeBarrier->SetForceRange);
	}
}

#undef TRY_SET_DOF_VAlUE
#undef TRY_SET_DOF_RANGE

#if WITH_EDITOR
void UAGX_ConstraintComponent::PostLoad()
{
	Super::PostLoad();
	BodyAttachment1.OnFrameDefiningComponentChanged(this);
	BodyAttachment2.OnFrameDefiningComponentChanged(this);

	for (FAGX_ConstraintBodyAttachment* BodyAttachment : {&BodyAttachment1, &BodyAttachment2})
	{
		if (BodyAttachment->FrameDefiningSource == EAGX_FrameDefiningSource::Other)
		{
			FAGX_SceneComponentReference* ComponentReference =
				&BodyAttachment->FrameDefiningComponent;
			ComponentReference->FallbackOwningActor = nullptr;
			/// \todo Investigate the relationship between FName("") and NAME_None, and what an
			/// emtpy text field in the Blueprint editor produces. Playing it safe for now and
			/// checking for both.
			if (ComponentReference->OwningActor == nullptr &&
				(ComponentReference->SceneComponentName != "" &&
				 ComponentReference->SceneComponentName != NAME_None))
			{
				// A nullptr FrameDefiningComponent actually means something (use the body as
				// origin), so we shouldn't set it unconditionally. We use the name as a sign that
				// an OwningActor should be set.
				ComponentReference->OwningActor = GetOwner();
				ComponentReference->CacheCurrentSceneComponent();
			}
		}
	}
}

void UAGX_ConstraintComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	BodyAttachment1.OnFrameDefiningComponentChanged(this);
	BodyAttachment2.OnFrameDefiningComponentChanged(this);
}

void UAGX_ConstraintComponent::BeginDestroy()
{
	Super::BeginDestroy();
	BodyAttachment1.OnDestroy(this);
	BodyAttachment2.OnDestroy(this);
}

void UAGX_ConstraintComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	if (DofGraphicsComponent1)
	{
		DofGraphicsComponent1->DestroyComponent();
	}
	if (DofGraphicsComponent2)
	{
		DofGraphicsComponent2->DestroyComponent();
	}
	if (IconGraphicsComponent)
	{
		IconGraphicsComponent->DestroyComponent();
	}
}
#endif

void UAGX_ConstraintComponent::BeginPlay()
{
	Super::BeginPlay();
	BodyAttachment1.RigidBody.CacheCurrentRigidBody();
	BodyAttachment2.RigidBody.CacheCurrentRigidBody();
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
