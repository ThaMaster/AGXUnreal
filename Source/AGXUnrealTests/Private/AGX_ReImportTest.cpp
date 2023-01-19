// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_ReImportTest_helpers
{
	bool CheckNodeNameAndEnsureNoParent(const UBlueprint& Blueprint, const FString& Name)
	{
		USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));

		if (Node == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find SCS Node '%s' in the Blueprint."), *Name);
			return false;
		}

		if (USCS_Node* Parent = Blueprint.SimpleConstructionScript->FindParentNode(Node))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("SCS Node '%s' has parent '%s' but was expected to not have a parent."), *Name,
				*Parent->GetName());
			return false;
		}

		return true;
	}

	bool CheckNodeNameAndParent(
		const UBlueprint& Blueprint, const FString& Name, const FString& ParentNodeName)
	{
		USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));
		if (Node == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find SCS Node '%s' in the Blueprint."), *Name);
			return false;
		}

		USCS_Node* Parent = Blueprint.SimpleConstructionScript->FindParentNode(Node);
		if (Parent == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("The SCS Node '%s' does not have a parent."), *Name);
			return false;
		}

		if (Parent->GetName() != ParentNodeName)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("The SCS Node '%s' has a parent Node named '%s', expected it to be '%s'."),
				*Name, *Parent->GetName(), *ParentNodeName);
			return false;
		}

		return true;
	}

	UBlueprint* Import(const FString& ArchiveFileName, bool IgnoreDisabledTrimeshes)
	{
		FString ArchiveFilePath =
			AgxAutomationCommon::GetTestScenePath(FPaths::Combine("ReImport", ArchiveFileName));
		if (ArchiveFilePath.IsEmpty())
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find an archive named '%s'."), *ArchiveFileName);
			return nullptr;
		}

		FAGX_ImportSettings ImportSettings;
		ImportSettings.bIgnoreDisabledTrimeshes = IgnoreDisabledTrimeshes;
		ImportSettings.bOpenBlueprintEditorAfterImport = false;
		ImportSettings.FilePath = ArchiveFilePath;
		ImportSettings.ImportType = EAGX_ImportType::Agx;

		return AGX_ImporterToBlueprint::Import(ImportSettings);
	}
}

//
// Common functionality
//
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FDeleteImportedAssets, FString, ArchiveName, FAutomationTestBase&, Test);

bool FDeleteImportedAssets::Update()
{
	const FString Root = FPaths::ProjectContentDir();
	const FString ImportsLocal =
		FPaths::Combine(FAGX_ImportUtilities::GetImportRootDirectoryName(), ArchiveName);
	const FString ImportsFull = FPaths::Combine(Root, ImportsLocal);
	const FString ImportsAbsolute = FPaths::ConvertRelativePathToFull(ImportsFull);
	if (!FPaths::DirectoryExists(ImportsAbsolute))
	{
		Test.AddError(FString::Printf(
			TEXT("Unable to delete files directory '%s' because it does not exist."),
			*ImportsAbsolute));
		return true;
	}

	if (!IFileManager::Get().DeleteDirectory(*ImportsAbsolute, true, true))
	{
		Test.AddError(FString::Printf(
			TEXT("IFileManager::DeleteDirectory returned false trying to remove: '%s'"),
			*ImportsAbsolute));
		return true;
	}

	return true;
}

//
// Re-import same twice test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FSameTwiceCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FSameTwiceCommand::Update()
{
	using namespace AGX_ReImportTest_helpers;

	UBlueprint* Blueprint = Import(ArchiveFileName, false);
	if (Blueprint == nullptr)
	{
		Test.AddError("Imported Blueprint was nullptr.");
		return true;
	}

	UBlueprint* BlueprintBase = FAGX_BlueprintUtilities::GetOutermostParent(Blueprint);
	if (BlueprintBase == nullptr)
	{
		Test.AddError(
			"Could not get Blueprint parent (base) from the returned Blueprint after import.");
		return true;
	}

	const FString SceneRootName =
		BlueprintBase->SimpleConstructionScript->GetDefaultSceneRootNode()->GetName();

	if (!CheckNodeNameAndParent(*BlueprintBase, "SphereBodyToChange", SceneRootName))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, "AGX_ReImport"))
		return true; // Logging done in CheckNodeNameAndEnsureNoParent.

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FReImportSameTwiceTest, "AGXUnreal.Editor.AGX_ReImportTest.ReImportSameTwice",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FReImportSameTwiceTest::RunTest(const FString& Parameters)
{
	UBlueprint* Blueprint = nullptr;
	const FString ArchiveFileName = "reimport_build.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FSameTwiceCommand(ArchiveFileName, *this));	
	ADD_LATENT_AUTOMATION_COMMAND(FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}
