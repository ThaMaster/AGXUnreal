#include "Shapes/AGX_ShapeComponent.h"

#include "AGX_LogCategory.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_MaterialInstance.h"
#include "Materials/MaterialBarrier.h"
#include "Utilities/AGX_StringUtilities.h"

// Sets default values for this component's properties
UAGX_ShapeComponent::UAGX_ShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	UE_LOG(LogAGX, Log, TEXT("ShapeComponent instance crated."));
}

bool UAGX_ShapeComponent::HasNative() const
{
	return GetNative() != nullptr;
}

void UAGX_ShapeComponent::UpdateVisualMesh()
{
	UE_LOG(LogAGX, Log, TEXT("Updating visual mesh of %s (of actor %s)"), *GetName(), *GetNameSafe(GetOwner()));

	ClearMeshData();

	TSharedPtr<FAGX_SimpleMeshData> Data(new FAGX_SimpleMeshData());

	if (ShouldCreateVisualMesh())
	{
		CreateVisualMesh(*Data.Get());
	}

	SetMeshData(Data);
}

bool UAGX_ShapeComponent::ShouldCreateVisualMesh() const
{
	return bVisible; // TODO: add && !(bHiddenInGame && IsGamePlaying), but how to get IsGamePlaying?
}

void UAGX_ShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	if (PhysicalMaterial)
	{
		UAGX_MaterialInstance* MaterialInstance = UAGX_MaterialBase::GetOrCreateInstance(GetWorld(), PhysicalMaterial);
		check(MaterialInstance);

		FMaterialBarrier* MaterialBarrier = MaterialInstance->GetOrCreateNative(GetWorld());
		check(MaterialBarrier);

		UE_LOG(
			LogAGX, Log,
			TEXT("UAGX_ShapeComponent::UpdateNativeProperties is setting native material \"%s\" on shape "
				 "\"%s\" of rigid body \"%s\"."),
			*MaterialInstance->GetName(), *GetName(), *GetNameSafe(GetOwner()));

		GetNative()->SetMaterial(*MaterialBarrier);
	}

	GetNative()->SetEnableCollisions(bCanCollide);
}

void UAGX_ShapeComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/// \todo Do we need TickComponent on UAGX_ShapeComponent?

	UE_LOG(LogAGX, Log, TEXT("TickComponent for ShapeComponent.")); // Haven't seen this in the log ??
}

#if WITH_EDITOR

bool UAGX_ShapeComponent::DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const
{
	return PropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bVisible) ||
		   PropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, RelativeScale3D);
}

void UAGX_ShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = GetFNameSafe(PropertyChangedEvent.Property);
	FName MemberPropertyName = GetFNameSafe(PropertyChangedEvent.MemberProperty);

	if (DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName))
	{
		UpdateVisualMesh();
	}
}

void UAGX_ShapeComponent::PostLoad()
{
	Super::PostLoad();

	UpdateVisualMesh();
}

void UAGX_ShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();

	UpdateVisualMesh();
}

#endif

// Called when the game starts
void UAGX_ShapeComponent::BeginPlay()
{
	UE_LOG(LogAGX, Log, TEXT("BeginPlay for ShapeComponent"));
	Super::BeginPlay();
	GetOrCreateNative();
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	UE_LOG(LogAGX, Log, TEXT("EndPlay for ShapeComponent"));
	Super::EndPlay(Reason);
	ReleaseNative();
}
