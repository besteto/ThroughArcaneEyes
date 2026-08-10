// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/TaeManaAttributeSet.h"

UTaeManaAttributeSet::UTaeManaAttributeSet()
{
	InitMana(100.f);
	InitMaxMana(100.f);
}

void UTaeManaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

bool UTaeManaAttributeSet::EvaluateExhaustion(const float Mana, const float MaxManaValue, const float RecoveryFraction, const bool bWasExhausted)
{
	if (!bWasExhausted)
	{
		return Mana <= 0.f;
	}

	const float RecoverAt = FMath::Clamp(RecoveryFraction, 0.f, 1.f) * MaxManaValue;
	return Mana < RecoverAt;
}
