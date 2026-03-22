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
