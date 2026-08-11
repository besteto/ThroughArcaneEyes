// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "UI/TaeHudViewModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaFlowTest,
	"ThroughArcaneEyes.UI.ManaFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaFlowTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("nothing active is idle"),
		UTaeHudViewModel::ResolveManaFlow(0, 0) == ETaeManaFlow::Idle);

	TestTrue(TEXT("one drain is draining"),
		UTaeHudViewModel::ResolveManaFlow(1, 0) == ETaeManaFlow::Draining);

	TestTrue(TEXT("one regen is regenerating"),
		UTaeHudViewModel::ResolveManaFlow(0, 1) == ETaeManaFlow::Regenerating);

	// Channelling adds a second drain on top of the vision drain — one ending must not clear the state
	TestTrue(TEXT("two drains still draining"),
		UTaeHudViewModel::ResolveManaFlow(2, 0) == ETaeManaFlow::Draining);

	// Regen is inhibited during Arcane, but if both are ever reported, spending wins visually
	TestTrue(TEXT("drain outranks regen"),
		UTaeHudViewModel::ResolveManaFlow(1, 1) == ETaeManaFlow::Draining);

	// Counts cannot go negative and drive a bogus state
	TestTrue(TEXT("negative counts read as idle"),
		UTaeHudViewModel::ResolveManaFlow(-1, 0) == ETaeManaFlow::Idle);

	return true;
}

#endif
