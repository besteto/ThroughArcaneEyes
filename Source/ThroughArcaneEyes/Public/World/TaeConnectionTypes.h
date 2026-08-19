// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "TaeConnectionTypes.generated.h"

// Lifecycle of one root connection. Growth is permanent and partial — a path that reaches Growing
// never falls back to Broken on its own, only when explicitly reset.
UENUM(BlueprintType)
enum class ETaeConnectionState : uint8
{
	Broken     UMETA(DisplayName = "Broken"),
	Growing    UMETA(DisplayName = "Growing"),
	Restored   UMETA(DisplayName = "Restored")
};

// Pure growth rules, deliberately free of actor/world dependencies so they can be tested directly.
struct THROUGHARCANEEYES_API FTaeGrowthStep
{
	// Adds DeltaAlpha to CurrentAlpha, clamped to [0,1].
	static float Advance(float CurrentAlpha, float DeltaAlpha);

	// Maps an alpha to its connection state.
	static ETaeConnectionState StateFor(float Alpha);
};
