// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "Misc/AutomationTest.h"
#include "GAS/TaeManaEffects.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaEffectsTest,
	"ThroughArcaneEyes.GAS.ManaEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaEffectsTest::RunTest(const FString& Parameters)
{
	// A periodic modifier applies once per period, so a per-second rate must be scaled down
	TestEqual(TEXT("ten per second over a tenth-second period"),
		UTaeManaEffectBase::MagnitudePerPeriod(10.f), 1.f);

	// Sign is carried through — callers pass negative rates for drains
	TestEqual(TEXT("negative rates stay negative"),
		UTaeManaEffectBase::MagnitudePerPeriod(-12.f), -1.2f);

	TestEqual(TEXT("zero is zero"),
		UTaeManaEffectBase::MagnitudePerPeriod(0.f), 0.f);

	// The period is the contract the magnitude depends on — pin it
	TestEqual(TEXT("period is a tenth of a second"),
		UTaeManaEffectBase::PeriodSeconds, 0.1f);

	return true;
}

#endif
