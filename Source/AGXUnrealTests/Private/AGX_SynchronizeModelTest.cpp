// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_SynchronizeModelTest_helpers
{
	// Child Blueprints, like the one produced after an Import does not necessarily contain any SCS
	// Nodes themselves. Instead one have to get the SCS Nodes from the base Blueprint, then get the
	// template Components from them, and go through the archetype instances to find the Components
	// of interest.

#if 0
	// todo: important; the found template Components from the GetTemplateComponents
	// call below are retrieved as expected. But calling GetArchetypeInstances on any of those components
	// gives nothing, which is really unexpected. It is just as if it is only from this test that
	// the issue exists, doing the anywhere in the Editor module of the plugin works as
	// expected.
	// Update: it seems that the archetype instances of the base Blueprint are created on-demand
	// when the Blueprint Editor is opened. This was confirmed by printing out the number of
	// archetype instances right after a regular Import but before the Blueprint Editor was opened.
	// The number was then zero.
	TArray<UActorComponent*> GetComponentsFromChildBlueprint(
		UBlueprint& BaseBp, UBlueprint& ChildBlueprint)
	{
		TArray<UActorComponent*> BaseComponents =
			FAGX_BlueprintUtilities::GetTemplateComponents(&BaseBp);

		TArray<UActorComponent*> ChildComponents;
		ChildComponents.Reserve(BaseComponents.Num());
		for (UActorComponent* BaseComponent : BaseComponents)
		{
			if (auto MatchedComponent =
					FAGX_ObjectUtilities::GetMatchedInstance(BaseComponent, &ChildBlueprint))
			{
				ChildComponents.Add(MatchedComponent);
			}
		}

		return ChildComponents;
	}
#endif

	//
	// High-level functions, working on entire models.
	//

	UBlueprint* Import(const FString& ArchiveFileName, bool IgnoreDisabledTrimeshes)
	{
		FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(
			FPaths::Combine(FString("SynchronizeModel"), ArchiveFileName));
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
			FPaths::Combine(FString("SynchronizeModel"), ArchiveFileName));
		if (ArchiveFilePath.IsEmpty())
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find an archive named '%s'."), *ArchiveFileName);
			return false;
		}

		FAGX_SynchronizeModelSettings Settigns;
		Settigns.bIgnoreDisabledTrimeshes = IgnoreDisabledTrimeshes;
		Settigns.FilePath = ArchiveFilePath;

		return AGX_ImporterToBlueprint::SynchronizeModel(BaseBp, Settigns);
	}

	//
	// Functions operating on the SCS node tree.
	//

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

	bool CheckNodeNonExisting(const UBlueprint& Blueprint, const FString& Name)
	{
		USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));
		if (Node != nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Found SCS Node '%s' that was expected not to exist in the Blueprint."),
				*Name);
			return false;
		}

		return true;
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

	bool CheckNodeNoChild(const UBlueprint& Blueprint, const FString& Name)
	{
		USCS_Node* Node = GetNodeChecked(Blueprint, Name);
		if (Node == nullptr)
			return false; // Logging/error done in GetNodeChecked.

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
				*Name, *Parent->GetVariableName().ToString(), *ParentNodeName);
			return false;
		}

		if (EnsureNoChild)
		{
			return CheckNodeNoChild(Blueprint, Name);
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
				LogAGX, Error,
				TEXT("Number of children of node '%s' was expected to be 1 but was %d."),
				*Node->GetVariableName().ToString(), Children.Num());
			return nullptr;
		}

		return Children[0];
	}

	//
	// Functions operating on template components.
	//

	template <typename UObject>
	UObject* GetTemplateComponentByName(TArray<UActorComponent*>& Components, const TCHAR* Name)
	{
		return AgxAutomationCommon::GetByName<UObject>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(Name));
	}

	template <typename UAsset>
	FString GetAssetPath(const FString& ArchiveName, const FString& AssetName)
	{
		const FString Path = FPaths::ConvertRelativePathToFull(FPaths::Combine(
			FPaths::ProjectContentDir(), FAGX_ImportUtilities::GetImportRootDirectoryName(),
			FPaths::GetBaseFilename(ArchiveName),
			FAGX_ImportUtilities::GetImportAssetDirectoryName<UAsset>(), AssetName));
		return Path;
	}

	//
	// Misc. functions.
	//

	template <typename F>
	F FromRad(F radians)
	{
		return FMath::RadiansToDegrees(radians);
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

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	// Doing a file system delete of the assets is a bit harsh. Works in this case since we know
	// nothing will use these assets the next time Unreal Editor is started since we don't use
	// the imported unit test assets for anything.
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

	auto NameEndsWithGuid = [](const FString& Name, const FString& StartToOmit)
	{
		FString GuidPart = Name;
		GuidPart.RemoveFromStart(StartToOmit);
		FGuid TestGuid(GuidPart);
		return TestGuid.IsValid();
	};

	// Ensure a GUID is part of the Node name for the collision Mesh as well as the render Mesh.
	// This is an important detail for the Model Synchronization pipeline. See comment in free
	// function SetUnnamedNameForPossibleCollisions in AGX_ImporterToBlueprint.cpp for more details.
	USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToChange");
	USCS_Node* TGTCCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
	USCS_Node* TGTCRenderMeshNode = GetOnlyAttachChildChecked(TGTCCollisionMeshNode);
	if (TGTCRenderMeshNode == nullptr)
	{
		Test.AddError("Render Mesh Component was nullptr. Cannot continue.");
		return true;
	}

	const FString CollisionMeshNamePreSync = TGTCCollisionMeshNode->GetVariableName().ToString();
	const FString RenderMeshNamePreSync = TGTCRenderMeshNode->GetVariableName().ToString();
	Test.TestTrue(
		"Collision Mesh Guid Naming", NameEndsWithGuid(CollisionMeshNamePreSync, "CollisionMesh_"));
	Test.TestTrue(
		"Render Mesh Guid Naming", NameEndsWithGuid(RenderMeshNamePreSync, "RenderMesh_"));

	if (!SynchronizeModel(*BlueprintBase, ArchiveFileName, false))
	{
		Test.AddError("SynchronizeModel returned false.");
		return true;
	}

	// Ensure the names for collision mesh / render mesh are still the same. These are tested
	// explicitly here to ensure naming conventions for those types are not changed without taking
	// into account the details outlined in SetUnnamedNameForPossibleCollisions in
	// AGX_ImporterToBlueprint.cpp.
	if (AgxAutomationCommon::IsAnyNullptr(TGTCCollisionMeshNode, TGTCRenderMeshNode))
	{
		Test.AddError(
			"Collision or render mesh Components was removed unexpectedly after model "
			"synchronization.");
		return true;
	}

	Test.TestEqual(
		"Collision Mesh name", TGTCCollisionMeshNode->GetVariableName().ToString(),
		CollisionMeshNamePreSync);

	Test.TestEqual(
		"Render Mesh name", TGTCRenderMeshNode->GetVariableName().ToString(),
		RenderMeshNamePreSync);

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
	// model synchronization. Note that a random GUID is appended after this, so we need to check
	// against the first part of the name.
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

/**
 * Import a model and simply synchronize against the same file as the original import. This is
 * somewhat a sanity-check test.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSynchronizeSameTest, "AGXUnreal.Editor.AGX_SynchronizeModelTest.SyncronizeSame",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FSynchronizeSameTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "synchronize_same_build.agx";
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
	using namespace AgxAutomationCommon;

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

	// Remember: Components in Blueprints have no attach parents setup. We need to check the SCS
	// Node tree for that information. We can do this by using the helper functions in the
	// AGX_SynchronizeModelTest_helpers namespace.

	// Important: we would like to get the components from the Blueprint child using the
	// GetComponentsFromChildBlueprint function in this file. Unfortunately that does not work while
	// Testing for some reason (see comment above that function). So we will have to look at the
	// base blueprint only.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(BlueprintBase);

	Test.TestTrue("Synchronized Components found.", Components.Num() > 0);

	// BodyToBeRenamed.
	{
		if (!CheckNodeNonExisting(*BlueprintBase, "BodyToBeRenamed"))
			return true; // Logging done in CheckNodeNonExisting.
	}

	// BodyWithNewName.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "BodyWithNewName", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.
	}

	// SphereBodyToBeRemoved and any children.
	{
		if (!CheckNodeNonExisting(*BlueprintBase, "SphereBodyToBeRemoved"))
			return true; // Logging done in CheckNodeNonExisting.

		if (!CheckNodeNonExisting(*BlueprintBase, "SphereGeometryToBeRemoved"))
			return true; // Logging done in CheckNodeNonExisting.
	}

	// SphereBodyToChange and any children.
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "SphereBodyToChange", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(
				*BlueprintBase, "SphereGeometryToChange", "SphereBodyToChange", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto BodyToChange = AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("SphereBodyToChange"));

		auto ShapeToChange = AgxAutomationCommon::GetByName<UAGX_SphereShapeComponent>(
			Components,
			*FAGX_BlueprintUtilities::ToTemplateComponentName("SphereGeometryToChange"));

		auto ShapeMat = ShapeToChange != nullptr ? ShapeToChange->ShapeMaterial : nullptr;

		auto MergeSplitThresholds =
			BodyToChange != nullptr ? BodyToChange->MergeSplitProperties.Thresholds : nullptr;

		if (IsAnyNullptr(BodyToChange, ShapeToChange, ShapeMat, MergeSplitThresholds))
		{
			Test.AddError(
				TEXT("At least one required objects owned by SphereBodyToChange was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual(
			"SphereBodyToChange location", AgxToUnrealDisplacement(-10.0, 1.0, 0.0),
			BodyToChange->GetRelativeLocation());
		Test.TestEqual("SphereBodyToChange mass", BodyToChange->GetMass(), 9.f);
		Test.TestEqual("SphereBodyToChange MST", MergeSplitThresholds->GetNormalAdhesion(), 132.0);

		Test.TestEqual(
			"SphereGeometryToChange location", AgxToUnrealDisplacement(0.0, 1.5, 0.0),
			ShapeToChange->GetRelativeLocation());
		Test.TestEqual(
			"SphereGeometryToChange radius", ShapeToChange->GetRadius(), AgxToUnrealDistance(0.25));
		Test.TestEqual("SharedMaterial roughness", ShapeMat->GetRoughness(), 0.23);
		Test.TestEqual(
			"SphereGeometryToChange num collision groups", ShapeToChange->CollisionGroups.Num(), 3);
		Test.TestTrue(
			"SphereGeometryToChange collision groups",
			ShapeToChange->CollisionGroups.Contains(FName("Sphere1")) &&
				ShapeToChange->CollisionGroups.Contains(FName("Sphere2")) &&
				ShapeToChange->CollisionGroups.Contains(FName("Sphere3")));
	}

	// BoxBodyToLooseGeom
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "BoxBodyToLooseGeom", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(
				*BlueprintBase, "ObserverToChangeOwner", "BoxBodyToLooseGeom", true))
			return true; // Logging done in CheckNodeNameAndParent.
	}

	// BoxBodyToGainGeom and any children.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "BoxBodyToGainGeom", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "BoxGeometryToMove", "BoxBodyToGainGeom", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Body = AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("BoxBodyToGainGeom"));
		auto Shape = AgxAutomationCommon::GetByName<UAGX_BoxShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("BoxGeometryToMove"));
		auto ShapeMat = Shape != nullptr ? Shape->ShapeMaterial : nullptr;

		if (IsAnyNullptr(Body, Shape, ShapeMat))
		{
			Test.AddError(
				TEXT("At least one required objects owned by BoxBodyToGainGeom was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual("BoxGeometryToMove num collision groups", Shape->CollisionGroups.Num(), 1);
		Test.TestTrue(
			"BoxGeometryToMove collision groups", Shape->CollisionGroups.Contains(FName("Box2")));
		Test.TestEqual("SharedMaterial roughness", ShapeMat->GetRoughness(), 0.23);

		// We also want to ensure that the BoxGeometryToMove and the SphereGeometryToChange Shape
		// Materials point to the same asset.
		auto ShapeToChange = AgxAutomationCommon::GetByName<UAGX_SphereShapeComponent>(
			Components,
			*FAGX_BlueprintUtilities::ToTemplateComponentName("SphereGeometryToChange"));
		auto ShapeMat2 = ShapeToChange != nullptr ? ShapeToChange->ShapeMaterial : nullptr;
		if (IsAnyNullptr(ShapeToChange, ShapeMat2))
		{
			Test.AddError(
				TEXT("At least one required objects for testing BoxBodyToGainGeom was nullptr, "
					 "cannot continue."));
			return true;
		}
		Test.TestEqual("Shared Shape Material same", ShapeMat, ShapeMat2);
	}

	// StandaloneCylToChange
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "StandaloneCylToChange", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Shape = AgxAutomationCommon::GetByName<UAGX_CylinderShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("StandaloneCylToChange"));
		auto ShapeMat = Shape != nullptr ? Shape->ShapeMaterial : nullptr;

		if (IsAnyNullptr(Shape, ShapeMat))
		{
			Test.AddError(
				TEXT("At least one required objects owned by StandaloneCylToChange was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual("CylinderMaterial roughness", ShapeMat->GetRoughness(), 0.24);
		Test.TestEqual(
			"StandaloneCylToChange height", Shape->GetHeight(), AgxToUnrealDistance(0.3f));
		Test.TestEqual(
			"StandaloneCylToChange location", AgxToUnrealDisplacement(-6.0, 1.0, 0.0),
			Shape->GetRelativeLocation());
		Test.TestEqual(
			"StandaloneCylToChange Shape Material Roughness", ShapeMat->GetRoughness(), 0.24);
		Test.TestTrue(
			"StandaloneCylToChange MSP",
			!Shape->MergeSplitProperties.bEnableMerge && !Shape->MergeSplitProperties.bEnableSplit);
	}

	// TrimeshBody and any children.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomToChange", "TrimeshBody", false))
			return true; // Logging done in CheckNodeNameAndParent.

		// Ensure correct attach parent/child tree under the Trimesh node.
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

		auto Shape = AgxAutomationCommon::GetByName<UAGX_TrimeshShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("TrimeshGeomToChange"));
		auto ShapeMat = Shape != nullptr ? Shape->ShapeMaterial : nullptr;
		auto RenderMesh = Cast<UStaticMeshComponent>(TGTCRenderMeshNode->ComponentTemplate);
		UMaterialInterface* RenderMat =
			RenderMesh != nullptr ? RenderMesh->GetMaterial(0) : nullptr;
		if (IsAnyNullptr(Shape, ShapeMat, RenderMesh, RenderMat))
		{
			Test.AddError(
				TEXT("At least one required objects owned by TrimeshBody was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual(
			"TrimeshGeomToChange location", AgxToUnrealDisplacement(0.0, 1.0, 0.0),
			Shape->GetRelativeLocation());

		// Check the Diffuse Color.
		FVector4 ExpectedLinear(0.6f, 0.14f, 0.01f, 1.0f);
		FMaterialParameterInfo Info;
		Info.Name = FName("Diffuse");
		FLinearColor ActualLinear;
		if (!RenderMat->GetVectorParameterValue(Info, ActualLinear, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get diffuse color from RenderMaterial '%s'."),
				*RenderMat->GetName()));
			return true;
		}

		FVector4 Actual = FAGX_ImportUtilities::LinearToSRGB(ActualLinear);
		float Tolerance = 1.0f / 255.0f; // This is all the precision we have in a byte.
		AgxAutomationCommon::TestEqual(
			Test, *FString::Printf(TEXT("%s in %s"), *Info.Name.ToString(), *RenderMat->GetName()),
			Actual, ExpectedLinear, Tolerance);
	}

	// TrimeshBodyToLooseGeom.
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "TrimeshBodyToLooseGeom", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.
	}

	// TrimeshBodyToGainGeom and any children.
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "TrimeshBodyToGainGeom", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(
				*BlueprintBase, "TrimeshGeomToMove", "TrimeshBodyToGainGeom", false))
			return true; // Logging done in CheckNodeNameAndParent.

		USCS_Node* TrimeshGeomToMoveNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToMove");
		USCS_Node* TGTMCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToMoveNode);
		USCS_Node* TGTMRenderMeshNode = GetOnlyAttachChildChecked(TGTMCollisionMeshNode);
		if (TGTMRenderMeshNode == nullptr)
		{
			Test.AddError("TGTMRenderMeshNode was nullptr");
			return true;
		}

		if (!CheckNodeNoChild(*BlueprintBase, TGTMRenderMeshNode->GetVariableName().ToString()))
			return true; // Logging done in CheckNodeNoChild.
	}

	// HingeToChange
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "HingeToChange", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Constraint = AgxAutomationCommon::GetByName<UAGX_HingeConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("HingeToChange"));
		if (Constraint == nullptr)
		{
			Test.AddError("HingeToChange was nullptr, cannot continue.");
			return true;
		}

		Test.TestEqual(
			"HingeToChange Compliance", Constraint->GetCompliance(EGenericDofIndex::Translational1),
			102.0);
		Test.TestEqual(
			"HingeToChange Body1", Constraint->BodyAttachment1.RigidBody.BodyName,
			FName("BodyWithNewName"));
	}

	// PrismaticToChange
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "PrismaticToChange", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Constraint = AgxAutomationCommon::GetByName<UAGX_PrismaticConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("PrismaticToChange"));
		if (Constraint == nullptr)
		{
			Test.AddError("PrismaticToChange was nullptr, cannot continue.");
			return true;
		}

		Test.TestEqual(
			"PrismaticToChange Damping", Constraint->GetSpookDamping(EGenericDofIndex::Rotational2),
			202.0);
	}

	// BallCToBeRemoved
	{
		if (!CheckNodeNonExisting(*BlueprintBase, "BallCToBeRemoved"))
			return true; // Logging done in CheckNodeNonExisting.
	}

	// NewHinge
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "NewHinge", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Constraint = AgxAutomationCommon::GetByName<UAGX_HingeConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("NewHinge"));
		if (Constraint == nullptr)
		{
			Test.AddError("NewHinge was nullptr, cannot continue.");
			return true;
		}

		Test.TestEqual(
			"NewHinge Body1", Constraint->BodyAttachment1.RigidBody.BodyName,
			FName("TrimeshBodyToGainGeom"));

		Test.TestEqual(
			"NewHinge Body2", Constraint->BodyAttachment2.RigidBody.BodyName,
			FName("TrimeshBodyToLooseGeom"));
	}

	// AGX_ModelSource.
	{
		if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, "AGX_ModelSource"))
			return true; // Logging done in CheckNodeNameAndEnsureNoParent.
	}

	// AGX_ContactmaterialRegistrar.
	{
		const FString CMRName = FAGX_ImportUtilities::GetContactMaterialRegistrarDefaultName();
		if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, CMRName))
			return true; // Logging done in CheckNodeNameAndEnsureNoParent.

		auto CMRegistrar = AgxAutomationCommon::GetByName<UAGX_ContactMaterialRegistrarComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(CMRName));
		if (CMRegistrar == nullptr)
		{
			Test.AddError("Contact Material Registrar was nullptr. Cannot continue.");
			return true;
		}

		Test.TestEqual("CMRegistrar num CM", CMRegistrar->ContactMaterials.Num(), 2);
		if (CMRegistrar->ContactMaterials.Num() == 2)
		{
			auto CmSharedCyl = CMRegistrar->ContactMaterials.FindByPredicate(
				[](auto* Cm) { return Cm->GetName().Contains("Shared"); });
			auto CmCylTri = CMRegistrar->ContactMaterials.FindByPredicate(
				[](auto* Cm) { return Cm->GetName().Contains("Trimesh"); });

			if (IsAnyNullptr(CmSharedCyl, CmCylTri))
			{
				Test.AddError(
					TEXT("At least one required Contact Material could not be found in the Contact "
						 "Material Registrar, cannot continue."));
				return true;
			}

			Test.TestEqual(
				"CmSharedCyl Friction Coefficient", (*CmSharedCyl)->GetFrictionCoefficient(), 0.12);
			Test.TestEqual(
				"CmCylTri Friction Coefficient", (*CmCylTri)->GetFrictionCoefficient(), 0.23);
		}
	}

	// AGX_CollisionGroupDisabler
	{
		const FString CGDName = FAGX_ImportUtilities::GetCollisionGroupDisablerDefaultName();
		if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, *CGDName))
			return true; // Logging done in CheckNodeNameAndEnsureNoParent.

		auto CGDisabler = AgxAutomationCommon::GetByName<UAGX_CollisionGroupDisablerComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(CGDName));
		if (CGDisabler == nullptr)
		{
			Test.AddError("Collision Group Disabler was nullptr. Cannot continue.");
			return true;
		}

		Test.TestEqual("CGDisabler num groups", CGDisabler->DisabledCollisionGroupPairs.Num(), 2);
		const TArray<FAGX_CollisionGroupPair> ExpectedGroups = {
			{"Sphere1", "Box2"}, {"Sphere2", "Sphere2"}};

		if (CGDisabler->DisabledCollisionGroupPairs.Num() == 2)
		{
			Test.TestTrue(
				"CGDisabler group 0",
				CGDisabler->DisabledCollisionGroupPairs[0].IsIn(ExpectedGroups));

			Test.TestTrue(
				"CGDisabler group 1",
				CGDisabler->DisabledCollisionGroupPairs[1].IsIn(ExpectedGroups));
		}
	}

	return true;
}

/**
 * Imports the large_model.agx file and then synchronizes against large_model_updated.agx where many
 * things are changed from the original.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSynchronizeLargeModelTest, "AGXUnreal.Editor.AGX_SynchronizeModelTest.SynchronizeLargeModel",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FSynchronizeLargeModelTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "large_model_build.agx";
	const FString UpdatedArchiveFileName = "large_model_updated_build.agx";
	ADD_LATENT_AUTOMATION_COMMAND(
		FSynchronizeLargeModelCommand(ArchiveFileName, UpdatedArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// IgnoreDisabledTrimesh false then true test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FIgnoreDisabledTrimeshFTCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FIgnoreDisabledTrimeshFTCommand::Update()
{
	using namespace AGX_SynchronizeModelTest_helpers;

	// Import with IgnoreDisabledTrimesh set to false.
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

	// Pre-synchronize.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomDisabled", "TrimeshBody", false))
			return true; // Logging done in CheckNodeNameAndParent.

		// Ensure correct attach parent/child tree under the Trimesh node.
		USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomDisabled");
		USCS_Node* CollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(CollisionMeshNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	// Synchronize with the IgnoreDisabledTrimesh setting true.
	if (!SynchronizeModel(*BlueprintBase, ArchiveFileName, true))
	{
		Test.AddError("SynchronizeModel returned false.");
		return true;
	}

	// Post-synchronize. Now the render mesh should be attached immediately under
	// the body.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNonExisting(*BlueprintBase, "TrimeshGeomDisabled"))
			return true; // Logging done in CheckNodeNonExisting.

		// Ensure correct attach parent/child tree under the Body node.
		USCS_Node* TrimeshBodyNode = GetNodeChecked(*BlueprintBase, "TrimeshBody");
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(TrimeshBodyNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	return true;
}

/**
 * Import model with a disabled Trimesh first with IgnoreDisabledTrimesh false, then synchronize
 * against the same model but this time with IgnoreDisabledTrimesh true.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIgnoreDisabledTrimeshFTTest,
	"AGXUnreal.Editor.AGX_SynchronizeModelTest.IgnoreDisabledTrimeshFT",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FIgnoreDisabledTrimeshFTTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "disabled_trimesh_ft.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FIgnoreDisabledTrimeshFTCommand(ArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// IgnoreDisabledTrimesh true then false test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FIgnoreDisabledTrimeshTFCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FIgnoreDisabledTrimeshTFCommand::Update()
{
	using namespace AGX_SynchronizeModelTest_helpers;

	// Import with IgnoreDisabledTrimesh true.
	UBlueprint* Blueprint = Import(ArchiveFileName, true);
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

	// Pre-synchronize. Now the collision mesh + render mesh should be attached immediately under
	// the body.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNonExisting(*BlueprintBase, "TrimeshGeomDisabled"))
			return true; // Logging done in CheckNodeNonExisting.

		// Ensure correct attach parent/child tree under the Body node.
		USCS_Node* TrimeshBodyNode = GetNodeChecked(*BlueprintBase, "TrimeshBody");
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(TrimeshBodyNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	// Synchronize with the the IgnoreDisabledTrimesh setting false.
	if (!SynchronizeModel(*BlueprintBase, ArchiveFileName, false))
	{
		Test.AddError("SynchronizeModel returned false.");
		return true;
	}

	// Post-synchronize. The Disabled Trimesh should now exist again.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomDisabled", "TrimeshBody", false))
			return true; // Logging done in CheckNodeNameAndParent.

		// Ensure correct attach parent/child tree under the Trimesh node.
		USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomDisabled");
		USCS_Node* CollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(CollisionMeshNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	return true;
}

/**
 * Import model with a disabled Trimesh first with IgnoreDisabledTrimesh true, then synchronize
 * against the same model but this time with IgnoreDisabledTrimesh false.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIgnoreDisabledTrimeshTFTest,
	"AGXUnreal.Editor.AGX_SynchronizeModelTest.IgnoreDisabledTrimeshTF",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FIgnoreDisabledTrimeshTFTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "disabled_trimesh_tf.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FIgnoreDisabledTrimeshTFCommand(ArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// Merge Split Thresholds synchronization test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FDeleteRemovedConstraintMergeSplitThresholdsCommand, FString, InitialArchiveName, FString,
	UpdatedArchiveFileName, FAutomationTestBase&, Test);

bool FDeleteRemovedConstraintMergeSplitThresholdsCommand::Update()
{
	using namespace AGX_SynchronizeModelTest_helpers;

	// Import initial state.
	UBlueprint* Blueprint = Import(InitialArchiveName, true);
	if (Blueprint == nullptr)
	{
		Test.AddError(FString::Printf(TEXT("Could not import '%s'."), *InitialArchiveName));
		return true;
	}

	// The Components live in the parent Blueprint, so get that.
	UBlueprint* BlueprintParent = FAGX_BlueprintUtilities::GetOutermostParent(Blueprint);
	if (BlueprintParent == nullptr)
	{
		Test.AddError(FString::Printf(
			TEXT("Could not get Blueprint parent for Blueprint imported from '%s'."),
			*InitialArchiveName));
		return true;
	}

	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(BlueprintParent);

	// Make sure we got the Components we expect.
	// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Hinge.
	if (!Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 4))
	{
		return true;
	}

	UAGX_HingeConstraintComponent* Hinge =
		AgxAutomationCommon::GetByName<UAGX_HingeConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(TEXT("Hinge")));
	if (!Test.TestNotNull(TEXT("Hinge Component with Merge Split Thresholds"), Hinge))
	{
		return true;
	}

	UAGX_ConstraintMergeSplitThresholds* Thresholds = Hinge->MergeSplitProperties.Thresholds;
	if (!Test.TestNotNull(TEXT("Thresholds on Hinge Component"), Thresholds))
	{
		return true;
	}

	// Check that the Thresholds asset file is where it should be.
	const FString AssetName =
		FString::Printf(TEXT("AGX_CMST_%s.uasset"), *Thresholds->ImportGuid.ToString());
	const FString AssetPath =
		GetAssetPath<UAGX_ConstraintMergeSplitThresholds>(InitialArchiveName, AssetName);
	if (!Test.TestTrue(TEXT("Thresholds asset exists"), FPaths::FileExists(AssetPath)))
	{
		return true;
	}

	// Synchronize with updated state.
	SynchronizeModel(*BlueprintParent, UpdatedArchiveFileName, true);

	// The thresholds should now be gone.
	Thresholds = Hinge->MergeSplitProperties.Thresholds;
	if (!Test.TestNull(TEXT("Thresholds"), Thresholds))
	{
		return true;
	}

	if (!Test.TestFalse(TEXT("Thresholds asset removed."), FPaths::FileExists(AssetPath)))
	{
		return true;
	}

	return true;


}

/**
 * Import model with a Rigid Body, a Hinge, and a Constraint Merge Split Thresholds. Then
 * synchronize with an updated model where the Hinge has been removed. The Constraint Merge Split
 * Thresholds should be removed with it.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeleteRemovedConstraintMergeSplitThresholdsTest,
	"AGXUnreal.Editor.AGX_SynchronizeModelTest.RemoveConstraintMergeSplitThresholds",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FDeleteRemovedConstraintMergeSplitThresholdsTest::RunTest(const FString& Parameters)
{
	const FString InitialArchiveFileName = "thresholds_remove__initial.agx";
	const FString UpdatedArchiveFileName = "thresholds_remove__updated.agx";
	const FString InitialArchiveName = FPaths::GetBaseFilename(InitialArchiveFileName);
	ADD_LATENT_AUTOMATION_COMMAND(FDeleteRemovedConstraintMergeSplitThresholdsCommand(
		InitialArchiveFileName, UpdatedArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(FDeleteImportedAssets(InitialArchiveName, *this));
	return true;
}
