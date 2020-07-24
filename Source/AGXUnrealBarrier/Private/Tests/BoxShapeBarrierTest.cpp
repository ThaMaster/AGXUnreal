
// AGXUnreal includes.
#include "Shapes/BoxShapeBarrier.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FBoxShapeBarrierTest,
  "Algoryx.AGXUnreal.Barrier",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::CriticalPriority | EAutomationTestFlags::ProductFilter)

bool FBoxShapeBarrierTest::RunTest(const FString& Parameters)
{
  return true;
}
