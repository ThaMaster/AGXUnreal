// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_ImportUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AMOR/AGX_ConstraintMergeSplitThresholds.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "AMOR/AGX_WireMergeSplitThresholds.h"
#include "AMOR/MergeSplitThresholdsBarrier.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "Shapes/RenderDataBarrier.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "AssetToolsModule.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Paths.h"
#include "RawMesh.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/ComponentEditorUtils.h"

namespace
{
	template <typename UAsset, typename FInitAssetCallback>
	FAssetToDiskInfo PrepareWriteAssetToDisk(
		const FString& DirectoryName, FString AssetName, const FString& FallbackName,
		const FString& AssetType, FInitAssetCallback InitAsset)
	{
		AssetName = FAGX_ImportUtilities::CreateAssetName(AssetName, FallbackName, AssetType);

		// If the asset name ends with lots of numbers then Unreal believes that
		// it is a counter starts looping trying to find the next available number,
		// which fails if the number is larger than the largest int32. This hack
		// twarts that by adding a useless character to the end of the name.
		int32 NumEndingNumerics = 0;
		for (int32 CharIndex = AssetName.Len() - 1; CharIndex >= 0; --CharIndex)
		{
			bool isNumeric = AssetName[CharIndex] >= TEXT('0') && AssetName[CharIndex] <= TEXT('9');
			if (!isNumeric)
				break;
			NumEndingNumerics++;
		}
		if (NumEndingNumerics >= 10)
		{
			AssetName = AssetName + "c";
			UE_LOG(
				LogAGX, Warning,
				TEXT("Asset '%s' was appended with a 'c' to avoid Unreal name processing bug."),
				*AssetName);
		}

		FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(DirectoryName, AssetType);
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#else
		UPackage* Package = CreatePackage(*PackagePath);
#endif

#if 0
		/// \todo Unclear if this is needed or not. Leaving it out for now but
		/// test with it restored if there are problems.
		Package->FullyLoad();
#endif
		UAsset* Asset = NewObject<UAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		if (!Asset)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Could not create asset '%s' from '%s'."), *AssetName,
				*DirectoryName);
			return FAssetToDiskInfo();
		}
		InitAsset(*Asset);

		return FAssetToDiskInfo {Package, Asset, PackagePath, AssetName};
	}

	bool WriteAssetToDisk(FAssetToDiskInfo& AtdInfo)
	{
		return FAGX_EditorUtilities::FinalizeAndSavePackage(AtdInfo);
	}
}

FString FAGX_ImportUtilities::CreatePackagePath(FString FileName, FString AssetType)
{
	FileName = FAGX_EditorUtilities::SanitizeName(FileName);
	AssetType = FAGX_EditorUtilities::SanitizeName(AssetType);
	if (FileName.IsEmpty())
	{
		return FString();
	}
	return FString::Printf(TEXT("/Game/%s/%s/%s/"), *GetImportRootDirectoryName(), *FileName, *AssetType);
}

FString FAGX_ImportUtilities::CreatePackagePath(FString FileName)
{
	FileName = FAGX_EditorUtilities::SanitizeName(FileName);
	if (FileName.IsEmpty())
	{
		return FString();
	}
	return FString::Printf(TEXT("/Game/%s/%s"), *GetImportRootDirectoryName(), *FileName);
}

FString FAGX_ImportUtilities::CreateAssetName(
	const FString& NativeName, const FString& FallbackName, const FString& AssetType)
{
	FString Name = FAGX_EditorUtilities::SanitizeName(NativeName);
	if (!Name.IsEmpty())
	{
		return Name;
	}
	Name = FAGX_EditorUtilities::SanitizeName(FallbackName);
	if (!Name.IsEmpty())
	{
		return Name;
	}
	return AssetType;
}

void FAGX_ImportUtilities::MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName)
{
	FString WantedPackageName = PackageName;
	FString WantedAssetName = AssetName;
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackageName, AssetName, PackageName, AssetName);
	if (AssetName != WantedAssetName)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Asset '%s' imported with name '%s' because of name conflict."),
			*WantedAssetName, *AssetName);
	}
}

namespace AGX_ImportUtilities_helpers
{
	template <typename FMeshFactory, typename FMeshDescription>
	void InitStaticMesh(
		FMeshFactory MeshFactory, const FMeshDescription& MeshDescription, UStaticMesh& Asset,
		bool bAllowCPUAccess)
	{
		FRawMesh RawMesh = MeshFactory(MeshDescription);
		FAGX_EditorUtilities::AddRawMeshToStaticMesh(RawMesh, &Asset);
		Asset.ImportVersion = EImportStaticMeshVersion::LastVersion;
		// Reading triangle data from a Static Mesh asset in a cooked build produces garbage on
		// Linux, which makes it impossible to create the corresponding AGX Dynamics Trimesh shape.
		// By setting this flag Unreal Engine will keep a copy of the triangle data in CPU memory
		// which we can read and create the Trimesh from.
		//
		// It comes with a memory cost, so once we have fixed the GPU copy problem the following
		// line should be removed.
		Asset.bAllowCPUAccess = bAllowCPUAccess;
	}
}

FAssetToDiskInfo FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryName, const FString& FallbackName)
{
	auto InitAsset = [&](UStaticMesh& Asset)
	{
		AGX_ImportUtilities_helpers::InitStaticMesh(
			&FAGX_EditorUtilities::CreateRawMeshFromTrimesh, Trimesh, Asset, true);
	};

	FString TrimeshSourceName = Trimesh.GetSourceName();
	if (TrimeshSourceName.Contains("\\") || TrimeshSourceName.Contains("/"))
	{
		TrimeshSourceName = FPaths::GetBaseFilename(TrimeshSourceName);
	}

	return PrepareWriteAssetToDisk<UStaticMesh>(
		DirectoryName, TrimeshSourceName, FallbackName, TEXT("StaticMesh"), InitAsset);
}

FAssetToDiskInfo FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FRenderDataBarrier& RenderData, const FString& DirectoryName)
{
	auto InitAsset = [&](UStaticMesh& Asset)
	{
		AGX_ImportUtilities_helpers::InitStaticMesh(
			&FAGX_EditorUtilities::CreateRawMeshFromRenderData, RenderData, Asset, true);
	};

	return PrepareWriteAssetToDisk<UStaticMesh>(
		DirectoryName, FString::Printf(TEXT("RenderMesh_%s"), *RenderData.GetGuid().ToString()),
		TEXT("RenderMesh"), TEXT("RenderMesh"), InitAsset);
}

namespace
{
	FString GetUniqueNameForComponentTemplate(const UActorComponent& Component, const FString& WantedName)
	{
		AGX_CHECK(FAGX_ObjectUtilities::IsTemplateComponent(Component));
		FString Name = WantedName;
		int suffix = 1;
		UBlueprint* Bp = FAGX_BlueprintUtilities::GetBlueprintFrom(Component);
		if (Bp == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to get Bluprint from Component '%s'. The final name may not be "
					 "correct."),
				*Component.GetName());
			return Component.GetName() + FGuid::NewGuid().ToString();
		}

		while (Bp->SimpleConstructionScript->FindSCSNode(FName(Name)) != nullptr)
		{
			Name = FString::Printf(TEXT("%s%d"), *WantedName, suffix++);
		}
		return Name;
	}

	// This is to some extent mimicking the behavior of
	// FComponentEditorUtils::GenerateValidVariableName but works for TemplateComponents which have
	// no owner Actor.
	FString GenerateValidVariableNameTemplateComponent(const UActorComponent& Component)
	{
		AGX_CHECK(FAGX_ObjectUtilities::IsTemplateComponent(Component));
		FString ComponentName =
			FBlueprintEditorUtils::GetClassNameWithoutSuffix(Component.GetClass());

		const FString SuffixToStrip(TEXT("Component"));
		if (ComponentName.EndsWith(SuffixToStrip))
		{
			ComponentName.LeftInline(ComponentName.Len() - SuffixToStrip.Len(), false);
		}

		UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(Component);
		if (Blueprint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to get Bluprint from Component '%s'. The final name may not be "
					 "correct."),
				*Component.GetName());
			return Component.GetName() + FGuid::NewGuid().ToString();
		}

		for (int i = 0; FAGX_BlueprintUtilities::NameExists(*Blueprint, ComponentName); i++)
		{
			ComponentName = FString::Printf(TEXT("%s%d"), *ComponentName, i);
		}
		return ComponentName;
	}

	/**
	 * Checks whether the component name is valid, and if not, generates a valid name and sets it to
	 * the component.
	 */
	FString GetFinalizedComponentName(UActorComponent& Component, const FString& WantedName)
	{
		if (Component.GetOwner() == nullptr &&
			!FAGX_ObjectUtilities::IsTemplateComponent(Component))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not find the owning actor of Actor Component: %s during name "
					 "finalization. The Component might not get the wanted name."),
				*Component.GetName());
			return FGuid::NewGuid().ToString();
		}

		if (!FComponentEditorUtils::IsValidVariableNameString(&Component, WantedName))
		{
			if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
			{
				return GenerateValidVariableNameTemplateComponent(Component);
			}
			else
			{
				return FComponentEditorUtils::GenerateValidVariableName(
					Component.GetClass(), Component.GetOwner());
			}
		}

		return WantedName;
	}
}

UMaterialInterface* FAGX_ImportUtilities::SaveImportedRenderMaterialAsset(
	const FAGX_RenderMaterial& Imported, const FString& DirectoryName, const FString& MaterialName)
{
	UMaterial* Base = LoadObject<UMaterial>(
		nullptr, TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'"));
	if (Base == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not load parent material for imported AGX Dynamics render materials."));
		return nullptr;
	}
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = Base;

	FString AssetName = FAGX_ImportUtilities::CreateAssetName(
		MaterialName, TEXT("ImportedAGXDynamicsMaterial"), TEXT("RenderMaterial"));
	FString PackagePath =
		FAGX_ImportUtilities::CreatePackagePath(DirectoryName, TEXT("RenderMaterial"));

	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackagePath, AssetName, PackagePath, AssetName);
	UObject* Asset = AssetTools.CreateAsset(
		AssetName, FPackageName::GetLongPackagePath(PackagePath),
		UMaterialInstanceConstant::StaticClass(), Factory);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not create new Material asset for material '%s' imported from '%s' "
				 "Falling back to the default imported material."),
			*MaterialName, *DirectoryName);
		return Base;
	}

	UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Asset);
	if (Material == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not create new Material Instance Constant for material '%s' imported from "
				 "'%s'. Falling back to the default imported material."),
			*MaterialName, *DirectoryName)
		return Base;
	}

	if (Imported.bHasDiffuse)
	{
		Material->SetVectorParameterValueEditorOnly(
			FName(TEXT("Diffuse")), FAGX_RenderMaterial::ConvertToLinear(Imported.Diffuse));
	}
	if (Imported.bHasAmbient)
	{
		Material->SetVectorParameterValueEditorOnly(
			FName(TEXT("Ambient")), FAGX_RenderMaterial::ConvertToLinear(Imported.Ambient));
	}
	if (Imported.bHasEmissive)
	{
		Material->SetVectorParameterValueEditorOnly(
			FName(TEXT("Emissive")), FAGX_RenderMaterial::ConvertToLinear(Imported.Emissive));
	}
	if (Imported.bHasShininess)
	{
		FMaterialParameterInfo Info;
		Info.Name = TEXT("Shininess");
		Material->SetScalarParameterValueEditorOnly(Info, Imported.Shininess);
	}

	Material->SetFlags(RF_Standalone);
	Material->MarkPackageDirty();
	Material->PostEditChange();
	UPackage* Package = Material->GetTypedOuter<UPackage>();
	if (Package != nullptr)
	{
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		UPackage::SavePackage(Package, Material, RF_NoFlags, *PackageFilename);
#else
		UPackage::SavePackage(Package, Material, *PackageFilename, FSavePackageArgs());
#endif
	}

	return Material;
}

UAGX_MergeSplitThresholdsBase* FAGX_ImportUtilities::SaveImportedMergeSplitAsset(
	const FMergeSplitThresholdsBarrier& Barrier, EAGX_AmorOwningType OwningType,
	const FString& DirectoryName, const FString& Name)
{
	switch (OwningType)
	{
		case EAGX_AmorOwningType::BodyOrShape:
		{
			auto InitAsset = [&](UAGX_ShapeContactMergeSplitThresholds& Asset)
			{ Asset.CopyFrom(Barrier); };

			FAssetToDiskInfo AtdInfo =
				PrepareWriteAssetToDisk<UAGX_ShapeContactMergeSplitThresholds>(
					DirectoryName, Name, "AGX_SMST_", TEXT("MergeSplitThresholds"), InitAsset);
			if (!WriteAssetToDisk(AtdInfo))
			{
				return nullptr;
			}
			return Cast<UAGX_ShapeContactMergeSplitThresholds>(AtdInfo.Asset);
		}
		case EAGX_AmorOwningType::Constraint:
		{
			auto InitAsset = [&](UAGX_ConstraintMergeSplitThresholds& Asset)
			{ Asset.CopyFrom(Barrier); };

			FAssetToDiskInfo AtdInfo = PrepareWriteAssetToDisk<UAGX_ConstraintMergeSplitThresholds>(
				DirectoryName, Name, "AGX_CMST_", TEXT("MergeSplitThresholds"), InitAsset);
			if (!WriteAssetToDisk(AtdInfo))
			{
				return nullptr;
			}
			return Cast<UAGX_ConstraintMergeSplitThresholds>(AtdInfo.Asset);

		}
		case EAGX_AmorOwningType::Wire:
		{
			auto InitAsset = [&](UAGX_WireMergeSplitThresholds& Asset) { Asset.CopyFrom(Barrier); };

			FAssetToDiskInfo AtdInfo = PrepareWriteAssetToDisk<UAGX_WireMergeSplitThresholds>(
				DirectoryName, Name, "AGX_WMST_", TEXT("MergeSplitThresholds"), InitAsset);
			if (!WriteAssetToDisk(AtdInfo))
			{
				return nullptr;
			}
			return Cast<UAGX_WireMergeSplitThresholds>(AtdInfo.Asset);
		}
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Could not create Merge Split Thresholds asset '%s' because the given owning type is "
			 "unknown."),
		*Name);
	return nullptr;
}

UAGX_TrackInternalMergeProperties*
FAGX_ImportUtilities::SaveImportedTrackInternalMergePropertiesAsset(
	const FTrackBarrier& Barrier, const FString& DirectoryName, const FString& Name)
{
	auto InitAsset = [&](UAGX_TrackInternalMergeProperties& Asset) { Asset.CopyFrom(Barrier); };

	FAssetToDiskInfo AtdInfo = PrepareWriteAssetToDisk<UAGX_TrackInternalMergeProperties>(
		DirectoryName, Name, TEXT(""), TEXT("TrackInternalMergeProperties"), InitAsset);
	if (!WriteAssetToDisk(AtdInfo))
	{
		return nullptr;
	}
	return Cast<UAGX_TrackInternalMergeProperties>(AtdInfo.Asset);
}

UAGX_TrackProperties* FAGX_ImportUtilities::SaveImportedTrackPropertiesAsset(
	const FTrackPropertiesBarrier& Barrier, const FString& DirectoryName, const FString& Name)
{
	auto InitAsset = [&](UAGX_TrackProperties& Asset) { Asset.CopyFrom(Barrier); };

	FAssetToDiskInfo AtdInfo = PrepareWriteAssetToDisk<UAGX_TrackProperties>(
		DirectoryName, Name, TEXT(""), TEXT("TrackProperties"), InitAsset);
	if (!WriteAssetToDisk(AtdInfo))
	{
		return nullptr;
	}
	return Cast<UAGX_TrackProperties>(AtdInfo.Asset);
}

FString FAGX_ImportUtilities::CreateName(UObject& Object, const FString& Name)
{
	if (Name.IsEmpty())
	{
		// Not having an imported name means use whatever default name Unreal decided.
		return Name;
	}
	if (Object.Rename(*Name, nullptr, REN_Test))
	{
		return Name;
	}
	else
	{
		FName NewName = MakeUniqueObjectName(Object.GetOuter(), Object.GetClass(), FName(*Name));
		UE_LOG(
			LogAGX, Log, TEXT("%s '%s' imported with name '%s' because of name conflict."),
			*Object.GetClass()->GetName(), *Name, *NewName.ToString());
		return NewName.ToString();
	}
}

void FAGX_ImportUtilities::Rename(UActorComponent& Component, const FString& Name)
{
	const FString ValidName = [&]()
	{
		if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
		{			
			return GetUniqueNameForComponentTemplate(Component, Name);
		}
		else
		{
			return CreateName(static_cast<UObject&>(Component), Name);
		}
	}();
	
	const FString FinalName = GetFinalizedComponentName(Component, ValidName);

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		FAGX_BlueprintUtilities::GetSCSNodeFromComponent(&Component)->SetVariableName(
			FName(FinalName), true);
	}
	else
	{
		Component.Rename(*FinalName);
	}
}

FLinearColor FAGX_ImportUtilities::SRGBToLinear(const FVector4& SRGB)
{
	FColor SRGBBytes(
		static_cast<uint8>(SRGB.X * 255.0f), static_cast<uint8>(SRGB.Y * 255.0f),
		static_cast<uint8>(SRGB.Z * 255.0f), static_cast<uint8>(SRGB.W * 255.0f));
	return {SRGBBytes};
}

FVector4 FAGX_ImportUtilities::LinearToSRGB(const FLinearColor& Linear)
{
	FColor SRGBBytes = Linear.ToFColor(true);
	return FVector4(
		static_cast<float>(SRGBBytes.R) / 255.0f, static_cast<float>(SRGBBytes.G) / 255.0f,
		static_cast<float>(SRGBBytes.B) / 255.0f, static_cast<float>(SRGBBytes.A) / 255.0f);
}

FString FAGX_ImportUtilities::GetImportRootDirectoryName()
{
	return FString("ImportedAGXModels");
}

FString FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName()
{
	return FString("ShapeMaterial");
}

FString FAGX_ImportUtilities::GetImportContactMaterialDirectoryName()
{
	return FString("ContactMaterial");
}
