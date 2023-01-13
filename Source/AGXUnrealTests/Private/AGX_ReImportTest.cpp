// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_ReImportTest_helpers
{
	
}

//
// Re-import same test starts here.
//

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FReImportSameTest, "AGXUnreal.Editor.AGX_ReImportTest.ReImportSame",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FReImportSameTest::RunTest(const FString& Parameters)
{
	TestTrue("To be done.", true);
	return true;
}
