#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"

// Unreal Engine includes.
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "UObject/UObjectIterator.h"

namespace AGX_ImporterToBlueprint_helpers
{
	void TestComponentsExists(
		const UBlueprint& Blueprint, const TArray<FString>& ExpectedNames,
		FAutomationTestBase& Test)
	{
		TArray<USCS_Node*> Nodes = Blueprint.SimpleConstructionScript->GetAllNodes();
		Test.TestEqual("Num components in Blueprint", Nodes.Num(), ExpectedNames.Num());

		for (const FString& Name : ExpectedNames)
		{
			// Append the _GEN_VARIABLE suffix.
			const FString TemplateName = Name + UActorComponent::ComponentTemplateNameSuffix;

			USCS_Node** Match = Nodes.FindByPredicate([TemplateName](USCS_Node* Node) {
				return Node->ComponentTemplate->GetName().Equals(TemplateName);
			});

			Test.TestNotNull(
				FString::Printf(
					TEXT("Expected component '%s' in Blueprint '%s'."), *TemplateName,
					*Blueprint.GetName()),
				Match);
		}
	}

	UActorComponent* GetFromWorld(const FString& Name)
	{
		for (TObjectIterator<UActorComponent> ClassItr; ClassItr; ++ClassItr)
		{
			if (*ClassItr == nullptr)
			{
				continue;
			}

			if (ClassItr->GetName().Equals(Name))
			{
				return *ClassItr;
			}
		}

		return nullptr;
	}

	TArray<UActorComponent*> GetComponents(const UBlueprint& Blueprint)
	{
		// @todo Find a proper way to directly get the components from the UBlueprint. Now we just
		// use ObjectIterator matching against the names of the Component templates in the Blueprint
		// which is not bulletproof.
		TArray<UActorComponent*> Components;
		TArray<USCS_Node*> Nodes = Blueprint.SimpleConstructionScript->GetAllNodes();

		for (USCS_Node* Node : Nodes)
		{
			FString Name = Node->ComponentTemplate->GetName();

			// Remove the _GEN_VARIABLE suffix.
			Name.RemoveFromEnd(UActorComponent::ComponentTemplateNameSuffix);
			if (UActorComponent* Match = GetFromWorld(Name))
			{
				Components.Add(Match);
			}
		}

		return Components;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
	FImportURDFToBlueprintCommand, FString, FileName, FString, PackagePath, UBlueprint*&, Blueprint,
	FAutomationTestBase&, Test);

bool FImportURDFToBlueprintCommand::Update()
{
	if (FileName.IsEmpty())
	{
		Test.AddError(TEXT("FImportURDFToBlueprintCommand not given a file to import."));
		return true;
	}
	FString FilePath = AgxAutomationCommon::GetTestScenePath(FileName);
	if (FilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find file '%s'."), *FilePath));
		return true;
	}
	Blueprint = AGX_ImporterToBlueprint::ImportURDF(FilePath, PackagePath);
	Test.TestNotNull(TEXT("Blueprint"), Blueprint);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FImportAGXArchiveToBlueprintCommand, FString, FileName, UBlueprint*&, Blueprint,
	FAutomationTestBase&, Test);

bool FImportAGXArchiveToBlueprintCommand::Update()
{
	if (FileName.IsEmpty())
	{
		Test.AddError(TEXT("FImportAGXArchiveToBlueprintCommand not given a file to import."));
		return true;
	}
	FString FilePath = AgxAutomationCommon::GetTestScenePath(FileName);
	if (FilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find file '%s'."), *FilePath));
		return true;
	}
	Blueprint = AGX_ImporterToBlueprint::ImportAGXArchive(FilePath);
	Test.TestNotNull(TEXT("Blueprint"), Blueprint);
	return true;
}

//
// URDF links geometries constraints test starts here.
//

class FImporterToBlueprint_URDFLinksGeometriesConstraintsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckURDFLinksGeometriesConstraintsImportedCommand,
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearURDFLinksGeometriesConstraintsImportedCommand,
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest&, Test);

class FImporterToBlueprint_URDFLinksGeometriesConstraintsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_URDFLinksGeometriesConstraintsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.URDFLinksGeometriesConstraints"))
	{
	}

public:
	UBlueprint* Blueprint = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportURDFToBlueprintCommand(
			TEXT("links_geometries_constraints.urdf"), TEXT(""), Blueprint, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckURDFLinksGeometriesConstraintsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearURDFLinksGeometriesConstraintsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest
		ImporterToBlueprint_URDFLinksGeometriesConstraintsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * file links_geometries_constraints.urdf.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckURDFLinksGeometriesConstraintsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	using namespace AGX_ImporterToBlueprint_helpers;

	if (Test.Blueprint == nullptr)
	{
		Test.AddError(
			TEXT("Could not import links_geometries_constraints test scene: No content created."));
		return true;
	}

	// clang-format off
	TArray<FString> ExpectedComponents {
		"DefaultSceneRoot",
		"boxlink",
		"boxcollision",
		"spherelink",
		"spherecollision",
		"cylinderlink",
		"cylindercollision",
		"freefallinglink",
		"freefallingcollision",
		"continuousjoint",
		"prismaticjoint"};
	// clang-format on

	TestComponentsExists(*Test.Blueprint, ExpectedComponents, Test);

	TArray<UActorComponent*> Components = GetComponents(*Test.Blueprint);
	UAGX_RigidBodyComponent* Boxlink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("boxlink"));
	UAGX_RigidBodyComponent* Shperelink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("spherelink"));
	UAGX_RigidBodyComponent* Cylinderlink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("cylinderlink"));
	UAGX_RigidBodyComponent* Freefallinglink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("freefallinglink"));

	Test.TestNotNull(TEXT("Boxlink"), Boxlink);
	Test.TestNotNull(TEXT("Shperelink"), Shperelink);
	Test.TestNotNull(TEXT("Cylinderlink"), Cylinderlink);
	Test.TestNotNull(TEXT("Freefallinglink"), Freefallinglink);

	if (Boxlink == nullptr || Shperelink == nullptr || Cylinderlink == nullptr ||
		Freefallinglink == nullptr)
	{
		Test.AddError("At least one Rigid Body was nullptr, cannot continue.");
		return true;
	}

	Test.TestEqual(
		TEXT("Boxlink position"), Boxlink->GetComponentLocation(),
		AgxToUnrealVector({0.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Shperelink position"), Shperelink->GetComponentLocation(),
		AgxToUnrealVector({1.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Cylinderlink position"), Cylinderlink->GetComponentLocation(),
		AgxToUnrealVector({2.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Freefallinglink position"), Freefallinglink->GetComponentLocation(),
		AgxToUnrealVector({0.f, 0.f, 0.f}));

	return true;
}

/**
 * Remove everything created by the import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearURDFLinksGeometriesConstraintsImportedCommand::Update()
{
	const TArray<const TCHAR*> FilesAndDirsToRemove = {
		TEXT("Blueprints"), TEXT("BP_links_geometries_constraints.uasset")};

	AgxAutomationCommon::DeleteImportDirectory(
		TEXT("links_geometries_constraints"), FilesAndDirsToRemove);
	return true;
}

//
// URDF link with meshes test starts here.
//

class FImporterToBlueprint_URDFLinkWithMeshesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckURDFLinkWithMeshesImportedCommand, FImporterToBlueprint_URDFLinkWithMeshesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearURDFLinkWithMeshesImportedCommand, FImporterToBlueprint_URDFLinkWithMeshesTest&, Test);

class FImporterToBlueprint_URDFLinkWithMeshesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_URDFLinkWithMeshesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_URDFLinkWithMeshesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.URDFLinkWithMeshes"))
	{
	}

public:
	UBlueprint* Blueprint = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportURDFToBlueprintCommand(
			TEXT("link_with_meshes.urdf"), AgxAutomationCommon::GetTestSceneDirPath(), Blueprint,
			*this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckURDFLinkWithMeshesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearURDFLinkWithMeshesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_URDFLinkWithMeshesTest ImporterToBlueprint_URDFLinkWithMeshesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * file link_with_meshes.urdf.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckURDFLinkWithMeshesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	using namespace AGX_ImporterToBlueprint_helpers;

	if (Test.Blueprint == nullptr)
	{
		Test.AddError(TEXT("Could not import link_with_meshes test scene: No content created."));
		return true;
	}

	TArray<UActorComponent*> Components = GetComponents(*Test.Blueprint);

	// One DefaultSceneRoot, one Rigid Body, one Trimesh with a render mesh and a collision mesh
	// and one Trimesh with only one collision mesh.
	Test.TestEqual("Number of components", Components.Num(), 7);

	UAGX_TrimeshShapeComponent* Urdfmeshvisual =
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("urdfmeshvisual"));
	UAGX_TrimeshShapeComponent* Urdfmeshcollision =
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("urdfmeshcollision"));


	Test.TestNotNull(TEXT("Urdfmeshvisual"), Urdfmeshvisual);
	Test.TestNotNull(TEXT("Urdfmeshcollision"), Urdfmeshcollision);

	Test.TestFalse("Urdfmeshvisual collide", Urdfmeshvisual->bCanCollide);
	Test.TestTrue("Urdfmeshcollision collide", Urdfmeshcollision->bCanCollide);

	return true;
}

/**
 * Remove everything created by the import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearURDFLinkWithMeshesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	using namespace AGX_ImporterToBlueprint_helpers;

	TArray<UActorComponent*> Components = GetComponents(*Test.Blueprint);
	TArray<FString> Assets = GetReferencedStaticMeshAssets(Components);
	if (Assets.Num() != 3)
	{
		Test.AddError(TEXT("Unexpected number of assets found."));
		return true;
	}

	TArray<const TCHAR*> FilesAndDirsToRemove;
	FilesAndDirsToRemove.Add(TEXT("Blueprints"));
	FilesAndDirsToRemove.Add(TEXT("BP_link_with_meshes.uasset"));
	FilesAndDirsToRemove.Add(TEXT("RenderMeshs"));
	FilesAndDirsToRemove.Add(TEXT("StaticMeshs"));
	for (const FString& Asset : Assets)
	{
		FilesAndDirsToRemove.Add(*Asset);
	}
		

	AgxAutomationCommon::DeleteImportDirectory(TEXT("link_with_meshes"), FilesAndDirsToRemove);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
