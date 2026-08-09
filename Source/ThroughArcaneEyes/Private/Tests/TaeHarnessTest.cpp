// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeHarnessTest,
	"ThroughArcaneEyes.Harness.Sanity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeHarnessTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("harness is wired up"), 1 + 1, 2);
	return true;
}

#endif
