// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "Misc/AutomationTest.h"
#include "World/TaeConnectionTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeGrowthStepTest,
	"ThroughArcaneEyes.World.GrowthStep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeGrowthStepTest::RunTest(const FString& Parameters)
{
	// Advance accumulates
	TestEqual(TEXT("advance from zero"), FTaeGrowthStep::Advance(0.f, 0.25f), 0.25f);
	TestEqual(TEXT("advance accumulates"), FTaeGrowthStep::Advance(0.25f, 0.25f), 0.5f);

	// Advance clamps at both ends — release-early must never overshoot or go negative
	TestEqual(TEXT("clamps at one"), FTaeGrowthStep::Advance(0.9f, 0.5f), 1.f);
	TestEqual(TEXT("clamps at zero"), FTaeGrowthStep::Advance(0.1f, -0.5f), 0.f);

	// State thresholds
	TestTrue(TEXT("zero is broken"),
		FTaeGrowthStep::StateFor(0.f) == ETaeConnectionState::Broken);
	TestTrue(TEXT("partial is growing"),
		FTaeGrowthStep::StateFor(0.5f) == ETaeConnectionState::Growing);
	TestTrue(TEXT("one is restored"),
		FTaeGrowthStep::StateFor(1.f) == ETaeConnectionState::Restored);

	// Boundary: anything above zero has begun, only exactly-full is restored
	TestTrue(TEXT("epsilon above zero is growing"),
		FTaeGrowthStep::StateFor(KINDA_SMALL_NUMBER * 2.f) == ETaeConnectionState::Growing);
	TestTrue(TEXT("tiny alpha below epsilon is growing"),
		FTaeGrowthStep::StateFor(0.00005f) == ETaeConnectionState::Growing);
	TestTrue(TEXT("just under one is still growing"),
		FTaeGrowthStep::StateFor(0.999f) == ETaeConnectionState::Growing);

	return true;
}

#endif
