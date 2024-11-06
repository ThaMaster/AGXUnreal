// Copyright 2024, Algoryx Simulation AB.

#include "Brick/AGX_BrickAssetCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Brick/AGX_BrickAsset.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "AssetSelection.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Blueprint.h"
#include "Engine/Level.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PackageTools.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_BrickAssetCustomization"

// IMPORTANT: This is all experimental test code used durint development. Remove before merge to
// master.

TSharedRef<IDetailCustomization> FAGX_BrickAssetCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_BrickAssetCustomization);
}

void FAGX_BrickAssetCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	const UAGX_BrickAsset* BrickAsset =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_BrickAsset>(InDetailBuilder);
	if (BrickAsset == nullptr)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Experiments");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("Import", "Import"))
						.ToolTipText(LOCTEXT("ImportTooltip",
							"Import using hard-coded path."))
						.OnClicked(this, &FAGX_BrickAssetCustomization::OnImportButtonClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("CreateBlueprints", "CreateBlueprints"))
						.ToolTipText(LOCTEXT("CreateBpsToolTip",
							"CreateBlueprints."))
						.OnClicked(this, &FAGX_BrickAssetCustomization::OnCreateBlueprintsButtonClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("UpdateBlueprint", "UpdateBlueprint"))
						.ToolTipText(LOCTEXT("UpdateBlueprintToolTip",
							"UpdateBlueprint."))
						.OnClicked(this, &FAGX_BrickAssetCustomization::OnUpdateBlueprintButtonClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("CopyBlueprint", "CopyBlueprint"))
						.ToolTipText(LOCTEXT("CopyBlueprintToolTip",
							"CopyBlueprint."))
						.OnClicked(this, &FAGX_BrickAssetCustomization::OnCopyFromOtherBlueprintButtonClicked)
					]
				]
			]
		]
	];
	// clang-format on
}

FReply FAGX_BrickAssetCustomization::OnImportButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	// FBrick::Test();
	return FReply::Handled();
}

namespace
{
	UPackage* CreateUPackage(const FString& BlueprintPackagePath)
	{
		UPackage* Package = CreatePackage(*BlueprintPackagePath);
		Package->FullyLoad();
		return Package;
	}

	FString CreatePackagePath(const FString& SubDir, FString Name)
	{
		const FString BasePath = FString(TEXT("/Game/"));
		FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(BasePath, SubDir);
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, Name);
		UPackage* ParentPackage = CreatePackage(*PackagePath);
		FString Path = FPaths::GetPath(ParentPackage->GetName());
		return UPackageTools::SanitizePackageName(Path + "/" + Name);
	}

	UBlueprint* CreateBlueprint(
		const FString& Name, AActor*& OutTemplate, bool SetNonDefaultMass = false)
	{
		GEditor->SelectNone(false, false);
		FString PackagePath = CreatePackagePath("Blueprint", Name);
		UPackage* Package = CreateUPackage(PackagePath);

		UActorFactory* Factory =
			GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
		FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
		UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
		OutTemplate = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
		check(OutTemplate != nullptr); /// \todo Test and return false instead of check?
		OutTemplate->SetFlags(RF_Transactional);
		OutTemplate->SetActorLabel(Name);

		UAGX_RigidBodyComponent* RigidBody =
			NewObject<UAGX_RigidBodyComponent>(OutTemplate, TEXT("Body"));
		RigidBody->bAutoGenerateMass = false;
		if (SetNonDefaultMass)
			RigidBody->Mass = 6.f;

		RigidBody->SetFlags(RF_Transactional);
		OutTemplate->AddInstanceComponent(RigidBody);
		RigidBody->RegisterComponent();
		RigidBody->PostEditChange();

		FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
		Params.bReplaceActor = false;
		Params.bKeepMobility = true;
		Params.bOpenBlueprint = false;

		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprintFromActor(
			Package->GetName(), OutTemplate, Params);

		// RootActorContainer->Destroy();
		FAGX_ObjectUtilities::SaveAsset(*Blueprint);

		// Child Blueprint
		FString PackagePathChild = CreatePackagePath("", FString::Printf(TEXT("%s_child"), *Name));
		UPackage* PackageChild = CreateUPackage(PackagePathChild);
		const FString AssetNameChild = FPaths::GetBaseFilename(PackageChild->GetName());

		UBlueprint* BlueprintChild = FKismetEditorUtilities::CreateBlueprint(
			Blueprint->GeneratedClass, PackageChild, FName(AssetNameChild),
			EBlueprintType::BPTYPE_Normal, UBlueprint::StaticClass(),
			UBlueprintGeneratedClass::StaticClass(), FName("HackyCodeCO"));

		GEngine->BroadcastLevelActorListChanged();

		FAGX_ObjectUtilities::SaveAsset(*BlueprintChild);

		return Blueprint;
	}
}

FReply FAGX_BrickAssetCustomization::OnCreateBlueprintsButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	UE_LOG(LogTemp, Warning, TEXT("CreateBlueprints!"));
	UBlueprint* BaseBlueprint = ::CreateBlueprint("BaseBlueprint", TemplateActor);
	BlueprintBase = BaseBlueprint;
	return FReply::Handled();
}

FReply FAGX_BrickAssetCustomization::OnUpdateBlueprintButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	AGX_CHECK(BlueprintBase);
	UE_LOG(LogTemp, Warning, TEXT("UpdateBlueprints!"));

	for (USCS_Node* Node : BlueprintBase->SimpleConstructionScript->GetAllNodes())
	{
		if (Node == nullptr)
			continue;

		UActorComponent* Component = Node->ComponentTemplate;

		if (auto Ri = Cast<UAGX_RigidBodyComponent>(Component))
		{
			Ri->Modify();
			Ri->Mass = 3.f;
			UE_LOG(LogTemp, Warning, TEXT("Updated rigidbody with new mass"));
			FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintBase);
			Ri->MarkPackageDirty();
			Ri->PostEditChange();
			FPropertyChangedEvent PropertyChangedEvent(Ri->GetClass()->FindPropertyByName(
				GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, Mass)));
			Ri->PostEditChangeProperty(PropertyChangedEvent);
		}
	}

	FKismetEditorUtilities::CompileBlueprint(BlueprintBase);
	FAGX_ObjectUtilities::SaveAsset(*BlueprintBase);

	return FReply::Handled();
}

FReply FAGX_BrickAssetCustomization::OnCopyFromOtherBlueprintButtonClicked()
{
#if 0 // didnt work
	AGX_CHECK(TemplateActor);
	AGX_CHECK(BlueprintBase);

	AActor* CDO = CastChecked<AActor>(BlueprintBase->GeneratedClass->GetDefaultObject());
	const EditorUtilities::ECopyOptions::Type CopyOptions = (EditorUtilities::ECopyOptions::Type)(
		EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances);
	EditorUtilities::CopyActorProperties(TemplateActor, CDO, CopyOptions);
#endif

	UBlueprint* BlueprintA = LoadObject<UBlueprint>(nullptr, TEXT("/Game/BlueprintA"));
	UBlueprint* BlueprintB = LoadObject<UBlueprint>(nullptr, TEXT("/Game/BlueprintB"));
	AGX_CHECK(BlueprintA != nullptr);
	AGX_CHECK(BlueprintB != nullptr);

#if 0 // Almost, kinda works if ReplaceStaleRefs is skipped with debugger
	//, but child blueprints will die, due to no parent anymore.
	UBlueprint* Replacement = FKismetEditorUtilities::ReplaceBlueprint(BlueprintB, BlueprintA);
	FKismetEditorUtilities::CompileBlueprint(Replacement);
	FAGX_ObjectUtilities::SaveAsset(*Replacement);
#endif

#if 0 // Crash when trying to open Blueprint afterwords.
	UEngine::CopyPropertiesForUnrelatedObjects(BlueprintA, BlueprintB);
#endif

#if 0 // does nothing
	AActor* CDOA = CastChecked<AActor>(BlueprintA->GeneratedClass->GetDefaultObject());
	AActor* CDOB = CastChecked<AActor>(BlueprintA->GeneratedClass->GetDefaultObject());
	UEngine::CopyPropertiesForUnrelatedObjects(CDOA, CDOB);
#endif

#if 0 // does nothing
	AActor* CDOA = CastChecked<AActor>(BlueprintA->GeneratedClass->GetDefaultObject());
	AActor* CDOB = CastChecked<AActor>(BlueprintA->GeneratedClass->GetDefaultObject());
	EditorUtilities::CopyActorProperties(CDOA, CDOB);
#endif

#if 0 // This will crash after CopyPropertiesForUnrelatedObjects.
	UBlueprint* NewBlueprint = ::CreateBlueprint("BaseBlueprint_new");
	// Now we have BlueprintBase and NewBlueprint which are almost identical.
	// NewBlueprint has another mass for the RigidBody.
	// Mission: can we somehow update BlueprintBase to match NewBlueprint?

	UEngine::CopyPropertiesForUnrelatedObjects(
		NewBlueprint->ParentClass, BlueprintBase->ParentClass);
	FKismetEditorUtilities::CompileBlueprint(BlueprintBase);
	FAGX_ObjectUtilities::SaveAsset(*BlueprintBase);
	FKismetEditorUtilities::CompileBlueprint(NewBlueprint);
	FAGX_ObjectUtilities::SaveAsset(*NewBlueprint);
#endif

#if 0 // Also crashes
	UEngine::CopyPropertiesForUnrelatedObjects(
		NewBlueprint->GeneratedClass, BlueprintBase->GeneratedClass);
#endif

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
