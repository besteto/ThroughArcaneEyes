// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "Misc/AutomationTest.h"
#include "World/TaeRootPath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeGrowthFrontTest,
	"ThroughArcaneEyes.World.GrowthFront",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeGrowthFrontTest::RunTest(const FString& Parameters)
{
	// The front sits where growth has reached
	TestEqual(TEXT("ungrown sits at the start"),
		ATaeRootPath::GrowthFrontDistance(0.f, 1000.f), 0.f);
	TestEqual(TEXT("half grown sits halfway"),
		ATaeRootPath::GrowthFrontDistance(0.5f, 1000.f), 500.f);
	TestEqual(TEXT("fully grown sits at the end"),
		ATaeRootPath::GrowthFrontDistance(1.f, 1000.f), 1000.f);

	// Alpha is clamped rather than running the front off either end of the spline
	TestEqual(TEXT("alpha above one clamps to the end"),
		ATaeRootPath::GrowthFrontDistance(2.f, 1000.f), 1000.f);
	TestEqual(TEXT("negative alpha clamps to the start"),
		ATaeRootPath::GrowthFrontDistance(-1.f, 1000.f), 0.f);

	// A degenerate spline cannot produce a negative distance
	TestEqual(TEXT("zero length yields zero"),
		ATaeRootPath::GrowthFrontDistance(0.5f, 0.f), 0.f);
	TestEqual(TEXT("negative length yields zero"),
		ATaeRootPath::GrowthFrontDistance(0.5f, -100.f), 0.f);

	return true;
}

#endif
