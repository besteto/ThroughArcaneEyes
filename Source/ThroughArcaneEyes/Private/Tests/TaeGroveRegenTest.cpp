// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "Misc/AutomationTest.h"
#include "World/TaeGroveComponent.h"
#include "Curves/CurveFloat.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeGroveRegenTest,
	"ThroughArcaneEyes.World.GroveRegen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeGroveRegenTest::RunTest(const FString& Parameters)
{
	// No curve means no regen, rather than a crash or an accidental free refill
	TestEqual(TEXT("null curve yields nothing"),
		UTaeGroveComponent::RegenRateForArea(100.f, nullptr), 0.f);

	UCurveFloat* Curve = NewObject<UCurveFloat>();
	Curve->FloatCurve.AddKey(50.f, 4.f);
	Curve->FloatCurve.AddKey(200.f, 10.f);

	// Authored points read back exactly
	TestEqual(TEXT("small grove"),
		UTaeGroveComponent::RegenRateForArea(50.f, Curve), 4.f);
	TestEqual(TEXT("large grove"),
		UTaeGroveComponent::RegenRateForArea(200.f, Curve), 10.f);

	// Outside the authored range the curve clamps rather than extrapolating away
	TestEqual(TEXT("below the first key clamps"),
		UTaeGroveComponent::RegenRateForArea(0.f, Curve), 4.f);
	TestEqual(TEXT("above the last key clamps"),
		UTaeGroveComponent::RegenRateForArea(1000.f, Curve), 10.f);

	// A degenerate footprint cannot produce negative regen
	TestTrue(TEXT("zero area is not negative"),
		UTaeGroveComponent::RegenRateForArea(0.f, Curve) >= 0.f);

	return true;
}

#endif
