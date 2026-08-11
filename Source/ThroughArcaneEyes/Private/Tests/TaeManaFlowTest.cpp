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
	// Pure resolver tests — validate the state resolution logic
	TestTrue(TEXT("resolver: nothing active is idle"),
		UTaeHudViewModel::ResolveManaFlow(0, 0) == ETaeManaFlow::Idle);

	TestTrue(TEXT("resolver: one drain is draining"),
		UTaeHudViewModel::ResolveManaFlow(1, 0) == ETaeManaFlow::Draining);

	TestTrue(TEXT("resolver: one regen is regenerating"),
		UTaeHudViewModel::ResolveManaFlow(0, 1) == ETaeManaFlow::Regenerating);

	// Channelling adds a second drain on top of the vision drain — one ending must not clear the state
	TestTrue(TEXT("resolver: two drains still draining"),
		UTaeHudViewModel::ResolveManaFlow(2, 0) == ETaeManaFlow::Draining);

	// Regen is inhibited during Arcane, but if both are ever reported, spending wins visually
	TestTrue(TEXT("resolver: drain outranks regen"),
		UTaeHudViewModel::ResolveManaFlow(1, 1) == ETaeManaFlow::Draining);

	// Counts cannot go negative and drive a bogus state
	TestTrue(TEXT("resolver: negative counts read as idle"),
		UTaeHudViewModel::ResolveManaFlow(-1, 0) == ETaeManaFlow::Idle);

	// Counter mutation tests — validate BeginManaFlow/EndManaFlow through the observable property
	UTaeHudViewModel* ViewModel = NewObject<UTaeHudViewModel>();
	if (!ViewModel)
	{
		return false;
	}

	// Test 1: The mapping is not swapped. Draining increments DrainCount, Regenerating increments RegenCount.
	ViewModel->BeginManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: begin drain results in draining state"),
		ViewModel->ManaFlow == ETaeManaFlow::Draining);

	ViewModel->EndManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: end drain results in idle state"),
		ViewModel->ManaFlow == ETaeManaFlow::Idle);

	ViewModel->BeginManaFlow(ETaeManaFlow::Regenerating);
	TestTrue(TEXT("instance: begin regen results in regenerating state"),
		ViewModel->ManaFlow == ETaeManaFlow::Regenerating);

	ViewModel->EndManaFlow(ETaeManaFlow::Regenerating);
	TestTrue(TEXT("instance: end regen results in idle state"),
		ViewModel->ManaFlow == ETaeManaFlow::Idle);

	// Test 2: Two simultaneous drains (Arcane Vision + Growth channel), one ending does not clear state
	ViewModel->BeginManaFlow(ETaeManaFlow::Draining);
	ViewModel->BeginManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: two drains is draining state"),
		ViewModel->ManaFlow == ETaeManaFlow::Draining);

	ViewModel->EndManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: first drain end still shows draining with second active"),
		ViewModel->ManaFlow == ETaeManaFlow::Draining);

	ViewModel->EndManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: second drain end returns to idle"),
		ViewModel->ManaFlow == ETaeManaFlow::Idle);

	// Test 3: Underflow through real methods — FMath::Max clamp prevents negative state
	ViewModel->EndManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: end drain when none active stays idle"),
		ViewModel->ManaFlow == ETaeManaFlow::Idle);

	ViewModel->BeginManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: after underflow, begin drain still produces draining (no negative residue)"),
		ViewModel->ManaFlow == ETaeManaFlow::Draining);

	ViewModel->EndManaFlow(ETaeManaFlow::Draining);
	TestTrue(TEXT("instance: state returns to idle after underflow recovery"),
		ViewModel->ManaFlow == ETaeManaFlow::Idle);

	return true;
}

#endif
