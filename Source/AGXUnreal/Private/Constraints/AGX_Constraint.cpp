// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/AGX_Constraint.h"

#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintComponent.h"

#include "Constraints/ConstraintBarrier.h"



EDofFlag ConvertDofsArrayToBitmask(const TArray<EDofFlag> &LockedDofsOrdered)
{
	uint8 bitmask(0);
	for (EDofFlag flag : LockedDofsOrdered)
	{
		bitmask |= (uint8)flag;
	}
	return (EDofFlag)bitmask;
}


TMap<EGenericDofIndex, int32> BuildNativeDofIndexMap(const TArray<EDofFlag> &LockedDofsOrdered)
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
			check(!"Should not reach this!");
		}
	}

	// General mappings:
	DofIndexMap.Add(EGenericDofIndex::ALL_DOF,  -1);

	return DofIndexMap;
}


AAGX_Constraint::AAGX_Constraint(const TArray<EDofFlag> &LockedDofsOrdered)
	:
bEnable(true),
SolveType(EAGX_SolveType::ST_DIRECT),
Elasticity(ConstraintConstants::DefaultElasticity(), ConvertDofsArrayToBitmask(LockedDofsOrdered)),
Damping(ConstraintConstants::DefaultDamping(), ConvertDofsArrayToBitmask(LockedDofsOrdered)),
ForceRange(ConstraintConstants::FloatRangeMin(), ConstraintConstants::FloatRangeMax(), ConvertDofsArrayToBitmask(LockedDofsOrdered)),
LockedDofs(LockedDofsOrdered),
NativeDofIndexMap(BuildNativeDofIndexMap(LockedDofsOrdered))
{
	ConstraintComponent = CreateDefaultSubobject<UAGX_ConstraintComponent>(
		TEXT("ConstraintComponent"));

	ConstraintComponent->SetFlags(ConstraintComponent->GetFlags() | RF_Transactional);
}


AAGX_Constraint::~AAGX_Constraint()
{

}


#if WITH_EDITOR

void AAGX_Constraint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() :
		NAME_None;

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() :
		NAME_None;

	if (MemberPropertyName == PropertyName) // Property of this class changed
	{
	}
	else // Property of an aggregate struct changed
	{
		FAGX_ConstraintBodyAttachment* ModifiedBodyAttachment = 
			(MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAGX_Constraint, BodyAttachment1)) ? &BodyAttachment1 :
			((MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAGX_Constraint, BodyAttachment2)) ? &BodyAttachment2 :
			nullptr);

		if (ModifiedBodyAttachment)
		{
			if (PropertyName == GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, FrameDefiningActor))
			{
				// TODO: Code below needs to be triggered also when modified through code!
				// Editor-only probably OK though, since it is just for Editor convenience.
				// See FCoreUObjectDelegates::OnObjectPropertyChanged.
				// Or/additional add Refresh button to AAGX_ConstraintFrameActor's Details Panel
				// that rebuilds the constraint usage list.
				ModifiedBodyAttachment->OnFrameDefiningActorChanged(this);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PostEditChangeProperty: PropertyName = %s, MemberPropertyName = %s"),
		*PropertyName.ToString(), *MemberPropertyName.ToString());
}

#endif


FConstraintBarrier* AAGX_Constraint::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}


FConstraintBarrier* AAGX_Constraint::GetNative()
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


bool AAGX_Constraint::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}


#define TRY_SET_DOF_VALUE(SourceStruct, GenericDof, Func) \
{ \
	int32 Dof; \
	if(ToNativeDof(GenericDof, Dof) && 0 <= Dof && Dof <= 5) \
		Func(SourceStruct[(int32)GenericDof], Dof); \
} \


#define TRY_SET_DOF_RANGE_VALUE(SourceStruct, GenericDof, Func) \
{ \
	int32 Dof; \
	if(ToNativeDof(GenericDof, Dof) && 0 <= Dof && Dof <= 5) \
		Func(SourceStruct[(int32)GenericDof].Min, SourceStruct[(int32)GenericDof].Max, Dof); \
} \


void AAGX_Constraint::UpdateNativeProperties()
{
	if (HasNative())
	{
		NativeBarrier->SetEnable(bEnable);
		NativeBarrier->SetSolveType(SolveType);

		// TODO: Could just loop NativeDofIndexMap instead!!

		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::TRANSLATIONAL_1, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::TRANSLATIONAL_2, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::TRANSLATIONAL_3, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::ROTATIONAL_1, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::ROTATIONAL_2, NativeBarrier->SetElasticity);
		TRY_SET_DOF_VALUE(Elasticity, EGenericDofIndex::ROTATIONAL_3, NativeBarrier->SetElasticity);

		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::TRANSLATIONAL_1, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::TRANSLATIONAL_2, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::TRANSLATIONAL_3, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::ROTATIONAL_1, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::ROTATIONAL_2, NativeBarrier->SetDamping);
		TRY_SET_DOF_VALUE(Damping, EGenericDofIndex::ROTATIONAL_3, NativeBarrier->SetDamping);

		TRY_SET_DOF_RANGE_VALUE(ForceRange, EGenericDofIndex::TRANSLATIONAL_1, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(ForceRange, EGenericDofIndex::TRANSLATIONAL_2, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(ForceRange, EGenericDofIndex::TRANSLATIONAL_3, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(ForceRange, EGenericDofIndex::ROTATIONAL_1, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(ForceRange, EGenericDofIndex::ROTATIONAL_2, NativeBarrier->SetForceRange);
		TRY_SET_DOF_RANGE_VALUE(ForceRange, EGenericDofIndex::ROTATIONAL_3, NativeBarrier->SetForceRange);
	}
}


#if WITH_EDITOR

void AAGX_Constraint::PostLoad()
{
	Super::PostLoad();
	BodyAttachment1.OnFrameDefiningActorChanged(this);
	BodyAttachment2.OnFrameDefiningActorChanged(this);
}


void AAGX_Constraint::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);	
	BodyAttachment1.OnFrameDefiningActorChanged(this);
	BodyAttachment2.OnFrameDefiningActorChanged(this);
}


void AAGX_Constraint::OnConstruction(const FTransform& Transform)
{
	Super::PostActorConstruction();
	BodyAttachment1.OnFrameDefiningActorChanged(this);
	BodyAttachment2.OnFrameDefiningActorChanged(this);
}


void AAGX_Constraint::BeginDestroy()
{
	Super::BeginDestroy();
	BodyAttachment1.OnDestroy(this);
	BodyAttachment2.OnDestroy(this);
}


void AAGX_Constraint::Destroyed()
{
	Super::Destroyed();
	BodyAttachment1.OnDestroy(this);
	BodyAttachment2.OnDestroy(this);
}
#endif


void AAGX_Constraint::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative())
	{
		CreateNative();
	}
}


bool AAGX_Constraint::ToNativeDof(EGenericDofIndex GenericDof, int32 &NativeDof)
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


void AAGX_Constraint::CreateNative()
{
	// TODO: Verify that we are in-game!

	check(!HasNative());
	
	CreateNativeImpl();

	// TODO: Shouldn't it be OK to continue if failed to initialize native (e.g. by lacking user setup)?
	// At least output a user error instead of crashing the program, since it is a user mistake
	// and not a code mistake to for example forgetting to assign a rigid body to the constraint!
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);

	Simulation->GetNative()->AddConstraint(NativeBarrier.Get());
}