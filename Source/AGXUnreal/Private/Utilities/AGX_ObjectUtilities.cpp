// Copyright 2023, Algoryx Simulation AB.

#include "Utilities/AGX_ObjectUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "UObject/SavePackage.h"

void FAGX_ObjectUtilities::GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors)
{
	TArray<AActor*> CurrentLevel;

	// Set Parent as root node of the tree
	CurrentLevel.Add(Parent);

	GetActorsTree(CurrentLevel, ChildActors);

	// Remove the parent itself from the ChildActors array
	ChildActors.Remove(Parent);
}

bool FAGX_ObjectUtilities::IsTemplateComponent(const UActorComponent& Component)
{
	return Component.HasAnyFlags(RF_ArchetypeObject);
}

void FAGX_ObjectUtilities::GetActorsTree(
	const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors)
{
	for (AActor* Actor : CurrentLevel)
	{
		if (Actor == nullptr)
		{
			continue;
		}

		ChildActors.Add(Actor);

		TArray<AActor*> NextLevel;
		Actor->GetAttachedActors(NextLevel);
		GetActorsTree(NextLevel, ChildActors);
	}
}

#if WITH_EDITOR
bool FAGX_ObjectUtilities::SaveAsset(UObject& Asset)
{
	UPackage* Package = Asset.GetPackage();
	if (Package == nullptr || Package->GetPathName().IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("SaveAsset called with Asset '%s' without a valid Package. The asset will not be "
				 "saved."),
			*Asset.GetName());
		return false;
	}

	Asset.MarkPackageDirty();
	Asset.PostEditChange();

	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		Asset.GetPackage()->GetPathName(), FPackageName::GetAssetPackageExtension());

	// A package must have meta-data in order to be saved. It seems to be created automatically
	// most of the time but sometimes, during unit tests for example, the engine tries to create it
	// on-demand while saving the package which leads to a fatal error because this type of object
	// look-up isn't allowed while saving packages. So try to force it here before calling
	// SavePackage.
	//
	// The error message sometimes printed while within UPackage::SavePackage called below is:
	// Illegal call to StaticFindObjectFast() while serializing object data or garbage collecting!
	Package->GetMetaData();

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	return UPackage::SavePackage(Package, &Asset, RF_NoFlags, *PackageFilename);
#else
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	return UPackage::SavePackage(Package, &Asset, *PackageFilename, SaveArgs);
#endif
}
#endif

FTransform FAGX_ObjectUtilities::GetAnyComponentWorldTransform(const USceneComponent& Component)
{
#if WITH_EDITOR
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		return FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(&Component);
	}
	else
	{
		return Component.GetComponentTransform();
	}
#else
	return Component.GetComponentTransform();
#endif
}

void FAGX_ObjectUtilities::SetAnyComponentWorldTransform(
	USceneComponent& Component, const FTransform& Transform, bool ForceOverwriteInstances)
{
#if WITH_EDITOR
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			&Component, Transform, true, ForceOverwriteInstances);
	}
	else
	{
		Component.SetWorldTransform(Transform);
	}
#else
	Component.SetWorldTransform(Transform);
#endif
}