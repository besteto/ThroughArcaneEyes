// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "TaeManaEffects.generated.h"

// Shared shape for every mana flow: an infinite effect that ticks a SetByCaller delta onto Mana.
// Carries no rates — callers supply the magnitude, so one class serves vision drain, growth drain,
// and grove regen at three different rates.
UCLASS(Abstract)
class THROUGHARCANEEYES_API UTaeManaEffectBase : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTaeManaEffectBase();

	// How often the effect ticks. Magnitudes are per-period, not per-second.
	static constexpr float PeriodSeconds = 0.1f;

	// Converts a designer-facing per-second rate into the per-period magnitude GAS actually applies.
	// Negative rates drain, positive rates restore.
	static float MagnitudePerPeriod(const float RatePerSecond) { return RatePerSecond * PeriodSeconds; }
};

// Applied by UGA_SpectralShift while Arcane Vision is active, and again by UGA_GrowRoot while
// channelling. Two applications tick independently, so the drains add up.
UCLASS()
class THROUGHARCANEEYES_API UTaeManaDrainEffect : public UTaeManaEffectBase
{
	GENERATED_BODY()

public:
	UTaeManaDrainEffect();
};

// Applied by UTaeGroveComponent while the player stands in a grove. Inhibited — not removed — while
// Arcane Vision is active, so recovery requires dropping back to Forest.
UCLASS()
class THROUGHARCANEEYES_API UTaeManaRegenEffect : public UTaeManaEffectBase
{
	GENERATED_BODY()

public:
	UTaeManaRegenEffect();
};
