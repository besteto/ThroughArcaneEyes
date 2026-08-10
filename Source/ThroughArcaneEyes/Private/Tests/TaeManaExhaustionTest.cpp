// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "GAS/TaeManaAttributeSet.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaExhaustionTest,
	"ThroughArcaneEyes.GAS.ManaExhaustion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaExhaustionTest::RunTest(const FString& Parameters)
{
	// Healthy mana never triggers exhaustion
	TestFalse(TEXT("half full is not exhausted"),
		UTaeManaAttributeSet::EvaluateExhaustion(50.f, 100.f, 0.25f, false));

	// Empty triggers it
	TestTrue(TEXT("empty becomes exhausted"),
		UTaeManaAttributeSet::EvaluateExhaustion(0.f, 100.f, 0.25f, false));

	// Once exhausted, a trickle is not enough — this is the hysteresis
	TestTrue(TEXT("stays exhausted below the floor"),
		UTaeManaAttributeSet::EvaluateExhaustion(10.f, 100.f, 0.25f, true));
	TestTrue(TEXT("stays exhausted just below the floor"),
		UTaeManaAttributeSet::EvaluateExhaustion(24.9f, 100.f, 0.25f, true));

	// Reaching the floor recovers
	TestFalse(TEXT("recovers at the floor"),
		UTaeManaAttributeSet::EvaluateExhaustion(25.f, 100.f, 0.25f, true));

	// A fraction above 1 is clamped rather than making recovery impossible
	TestFalse(TEXT("fraction above one clamps to full"),
		UTaeManaAttributeSet::EvaluateExhaustion(100.f, 100.f, 2.f, true));

	// A zero fraction means any mana at all recovers
	TestFalse(TEXT("zero fraction recovers immediately"),
		UTaeManaAttributeSet::EvaluateExhaustion(0.f, 100.f, 0.f, true));

	return true;
}

#endif
