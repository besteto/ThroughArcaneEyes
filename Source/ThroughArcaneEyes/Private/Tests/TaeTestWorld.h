// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

// A real UWorld for tests that need one. Some behaviour cannot be reached by a pure function —
// periodic Gameplay Effects need a ticking world, and a Material Parameter Collection has no
// instance without one.
namespace Tae::Test
{
	// Mirrors the engine's own GAS tests (GameplayEffectTests.cpp:761): sub-tick in fixed steps so
	// periodic effects land, and bump GFrameCounter because GAS caches per-frame state.
	inline void TickWorld(UWorld* World, float Seconds)
	{
		constexpr float Step = 0.1f;
		while (Seconds > 0.f)
		{
			World->Tick(ELevelTick::LEVELTICK_All, FMath::Min(Seconds, Step));
			Seconds -= Step;
			GFrameCounter++;
		}
	}

	// Setup and teardown matching the engine's GAS tests (GameplayEffectTests.cpp:849, 865) so the
	// test leaves no world context behind.
	struct FScopedTestWorld
	{
		FScopedTestWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			WorldContext.SetCurrentWorld(World);

			FURL URL;
			World->InitializeActorsForPlay(URL);
			World->BeginPlay();

			InitialFrameCounter = GFrameCounter;
		}

		~FScopedTestWorld()
		{
			GFrameCounter = InitialFrameCounter;
			World->EndPlay(EEndPlayReason::Quit);
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UWorld* World = nullptr;
		uint64 InitialFrameCounter = 0;
	};
}

#endif
