// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "Misc/AutomationTest.h"
#include "Core/TaeArcaneSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeBlendAlphaTest,
	"ThroughArcaneEyes.Core.BlendAlpha",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeBlendAlphaTest::RunTest(const FString& Parameters)
{
	// Half the duration covers half the distance
	TestEqual(TEXT("half step toward one"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.f, 1.f, 0.25f, 0.5f), 0.5f);

	// Never overshoots the target
	TestEqual(TEXT("clamps to target going up"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.9f, 1.f, 1.f, 0.5f), 1.f);
	TestEqual(TEXT("clamps to target going down"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.1f, 0.f, 1.f, 0.5f), 0.f);

	// Already at target is a no-op
	TestEqual(TEXT("at target stays"),
		UTaeArcaneSubsystem::StepBlendAlpha(1.f, 1.f, 0.016f, 0.5f), 1.f);

	// Zero or negative duration snaps rather than dividing by zero
	TestEqual(TEXT("zero duration snaps"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.f, 1.f, 0.016f, 0.f), 1.f);

	return true;
}

#endif
