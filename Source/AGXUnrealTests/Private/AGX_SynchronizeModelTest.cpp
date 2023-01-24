// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_SynchronizeModelTest_helpers
{
	USCS_Node* GetNodeChecked(const UBlueprint& Blueprint, const FString& Name)
	{
		USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));
		if (Node == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find SCS Node '%s' in the Blueprint."), *Name);
			return nullptr;
		}

		return Node;
	}

	bool CheckNodeNameAndEnsureNoParent(const UBlueprint& Blueprint, const FString& Name)
	{
		USCS_Node* Node = GetNodeChecked(Blueprint, Name);
		if (Node == nullptr)
			return false;

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

	bool CheckNoChild(const UBlueprint& Blueprint, const FString& Name)
	{
		USCS_Node* Node = GetNodeChecked(Blueprint, Name);
		if (Node == nullptr)
			return false;

		if (Node->GetChildNodes().Num() != 0)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Expected node '%s' not to have zero children, but it had %d"),
				*Name, Node->GetChildNodes().Num());
			return false;
		}

		return true;
	}

	bool CheckNodeNameAndParent(
		const UBlueprint& Blueprint, const FString& Name, const FString& ParentNodeName,
		bool EnsureNoChild)
	{
		USCS_Node* Node = GetNodeChecked(Blueprint, Name);
		if (Node == nullptr)
			return false;

		USCS_Node* Parent = Blueprint.SimpleConstructionScript->FindParentNode(Node);
		if (Parent == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("The SCS Node '%s' does not have a parent."), *Name);
			return false;
		}

		if (Parent->GetVariableName().ToString() != ParentNodeName)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("The SCS Node '%s' has a parent Node named '%s', expected it to be '%s'."),
				*Name, *Parent->GetName(), *ParentNodeName);
			return false;
		}

		if (EnsureNoChild)
		{
			return CheckNoChild(Blueprint, Name);
		}

		return true;
	}

	USCS_Node* GetOnlyAttachChildChecked(USCS_Node* Node)
	{
		if (Node == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOnlyAttachChildChecked failed because the passed Node was nullptr."));
			return nullptr;
		}

		const auto& Children = Node->GetChildNodes();
		if (Children.Num() != 1)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Number of children of node '%s' was expected to be 1 but was %d."),
				*Node->GetVariableName().ToString(), Children.Num());
			return nullptr;
		}

		return Children[0];
	}

	UBlueprint* Import(const FString& ArchiveFileName, bool IgnoreDisabledTrimeshes)
	{
		FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(
			FPaths::Combine("SynchronizeModel", ArchiveFileName));
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

	bool SynchronizeModel(
		UBlueprint& BaseBp, const FString& ArchiveFileName, bool IgnoreDisabledTrimeshes)
	{
		FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(
			FPaths::Combine("SynchronizeModel", ArchiveFileName));
		if (ArchiveFilePath.IsEmpty())
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find an archive named '%s'."), *ArchiveFileName);
			return false;
		}

		FAGX_ImportSettings ImportSettings;
		ImportSettings.bIgnoreDisabledTrimeshes = IgnoreDisabledTrimeshes;
		ImportSettings.bOpenBlueprintEditorAfterImport = false;
		ImportSettings.FilePath = ArchiveFilePath;
		ImportSettings.ImportType = EAGX_ImportType::Agx;

		return AGX_ImporterToBlueprint::SynchronizeModel(BaseBp, ImportSettings);
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
// Synchronize same twice test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FSynchronizeSameCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FSynchronizeSameCommand::Update()
{
	using namespace AGX_SynchronizeModelTest_helpers;

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

	const int NumNodesFirstImport = BlueprintBase->SimpleConstructionScript->GetAllNodes().Num();

	if (!SynchronizeModel(*BlueprintBase, ArchiveFileName, false))
	{
		Test.AddError("SynchronizeModel returned false.");
		return true;
	}

	// Ensure we have the same number of nodes after the model Synchronization as before.
	const int NumNodesSynchronizeModel =
		BlueprintBase->SimpleConstructionScript->GetAllNodes().Num();
	if (NumNodesSynchronizeModel != NumNodesFirstImport)
	{
		Test.AddError(FString::Printf(
			TEXT("Number of nodes was %d after import and %d after model synchronization. "
				 "They are expected to be the same."),
			NumNodesFirstImport, NumNodesSynchronizeModel));
		return true;
	}

	// Ensure no nodes have the name "AGX_Import_Unnamed..." name that is set in the beginning of a
	// model synchronization.
	const FString UnnamedName = "AGX_Import_Unnamed";
	const FString UnsetUniqueImportName = FAGX_ImportUtilities::GetUnsetUniqueImportName();
	Test.TestTrue(
		"Unexpected Unset Unique Import Name unexpected",
		UnsetUniqueImportName.StartsWith(UnnamedName));
	for (USCS_Node* Node : BlueprintBase->SimpleConstructionScript->GetAllNodes())
	{
		Test.TestFalse("Invalid name", Node->GetVariableName().ToString().StartsWith(UnnamedName));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSynchronizeSameTest, "AGXUnreal.Editor.AGX_SynchronizeModelTest.SyncronizeSame",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FSynchronizeSameTest::RunTest(const FString& Parameters)
{
	UBlueprint* Blueprint = nullptr;
	const FString ArchiveFileName = "large_model_build.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FSynchronizeSameCommand(ArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// Synchronize large model test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FSynchronizeLargeModelCommand, FString, ArchiveFileName, FString, UpdatedArchiveFileName,
	FAutomationTestBase&, Test);

bool FSynchronizeLargeModelCommand::Update()
{
	using namespace AGX_SynchronizeModelTest_helpers;

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

	if (!SynchronizeModel(*BlueprintBase, UpdatedArchiveFileName, false))
	{
		Test.AddError("SynchronizeModel returned false.");
		return true;
	}

	if (!CheckNodeNameAndParent(*BlueprintBase, "SphereBodyToChange", "DefaultSceneRoot", false))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(
			*BlueprintBase, "SphereGeometryToChange", "SphereBodyToChange", true))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "SphereBodyNoChange", "DefaultSceneRoot", false))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(
			*BlueprintBase, "SphereGeometryNoChange", "SphereBodyNoChange", true))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "BoxBodyToLooseGeom", "DefaultSceneRoot", false))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "BoxGeometryToMove", "BoxBodyToLooseGeom", true))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "BoxBodyToGainGeom", "DefaultSceneRoot", true))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "StandaloneCylToChange", "DefaultSceneRoot", true))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomToChange", "TrimeshBody", false))
		return true; // Logging done in CheckNodeNameAndParent.

	{
		USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToChange");
		USCS_Node* TGTCCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
		USCS_Node* TGTCRenderMeshNode = GetOnlyAttachChildChecked(TGTCCollisionMeshNode);
		if (TGTCRenderMeshNode == nullptr)
		{
			Test.AddError("TGTCRenderMeshNode was nullptr");
			return true;
		}

		if (TGTCRenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected TGTCRenderMeshNode to have zero children but it has %d."),
				TGTCRenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	if (!CheckNodeNameAndParent(
			*BlueprintBase, "TrimeshBodyToLooseGeom", "DefaultSceneRoot", false))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndParent(
			*BlueprintBase, "TrimeshGeomToMove", "TrimeshBodyToLooseGeom", false))
		return true; // Logging done in CheckNodeNameAndParent.

	{
		USCS_Node* TrimeshGeomToMoveNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToMove");
		USCS_Node* TGTMCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToMoveNode);
		USCS_Node* TGTMRenderMeshNode = GetOnlyAttachChildChecked(TGTMCollisionMeshNode);
		if (TGTMRenderMeshNode == nullptr)
		{
			Test.AddError("TGTMRenderMeshNode was nullptr");
			return true;
		}

		if (TGTMRenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected TGTMRenderMeshNode to have zero children but it has %d."),
				TGTMRenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBodyToGainGeom", "DefaultSceneRoot", true))
		return true; // Logging done in CheckNodeNameAndParent.

	if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, "AGX_ModelSource"))
		return true; // Logging done in CheckNodeNameAndEnsureNoParent.

	return true;
}

/**
 * Imports the large_model.agx file and then synchronizes against large_model_updated.agx where many
 * things are changed from the original.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSynchronizeLargeModelTest, "AGXUnreal.Editor.AGX_SynchronizeModelTest.SyncronizeLargeModel",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FSynchronizeLargeModelTest::RunTest(const FString& Parameters)
{
	UBlueprint* Blueprint = nullptr;
	const FString ArchiveFileName = "large_model_build.agx";
	const FString UpdatedArchiveFileName = "large_model_updated_build.agx";
	ADD_LATENT_AUTOMATION_COMMAND(
		FSynchronizeLargeModelCommand(ArchiveFileName, UpdatedArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}
