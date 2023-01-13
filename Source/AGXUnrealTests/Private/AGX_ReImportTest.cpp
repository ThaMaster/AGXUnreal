// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"

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
}

//
// Re-import same twice test starts here.
//

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FReImportSameTwiceTest, "AGXUnreal.Editor.AGX_ReImportTest.ReImportSameTwice",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FReImportSameTwiceTest::RunTest(const FString& Parameters)
{
	using namespace AGX_ReImportTest_helpers;

	const FString ArchiveFileName = "same_twice_build.agx";
	FString ArchiveFilePath =
		AgxAutomationCommon::GetTestScenePath(FPaths::Combine("ReImport", ArchiveFileName));
	if (ArchiveFilePath.IsEmpty())
	{
		AddError(FString::Printf(TEXT("Did not find an archive named '%s'."), *ArchiveFileName));
		return true;
	}

	FAGX_ImportSettings ImportSettings;
	ImportSettings.bIgnoreDisabledTrimeshes = false;
	ImportSettings.bOpenBlueprintEditorAfterImport = false;
	ImportSettings.FilePath = ArchiveFilePath;
	ImportSettings.ImportType = EAGX_ImportType::Agx;

	UBlueprint* BlueprintChild = AGX_ImporterToBlueprint::Import(ImportSettings);
	if (BlueprintChild == nullptr)
	{
		AddError("The Blueprint returned after import was nullptr.");
	}

	UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetOutermostParent(BlueprintChild);
	if (Blueprint == nullptr)
	{
		AddError("Could not get Blueprint parent (base) from the returned Blueprint after import.");
	}

	const FString SceneRootName =
		Blueprint->SimpleConstructionScript->GetDefaultSceneRootNode()->GetName();

	if (!CheckNodeNameAndParent(*Blueprint, "SphereBody", SceneRootName))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndEnsureNoParent(*Blueprint, "AGX_ReImport"))
		return true; // Logging done in CheckNodeNameAndEnsureNoParent.

	// Todo: Add more stuff to the test scene, and also re-import and check content the same way again.

	return true;
}
