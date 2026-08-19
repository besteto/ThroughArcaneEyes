// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "GAS/TaeManaAttributeSet.h"
#include "GAS/TaeGASTypes.h"

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

void UTaeManaAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// MaxMana matters too — it moves the recovery floor
	if (Attribute != GetManaAttribute() && Attribute != GetMaxManaAttribute())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const bool bWasExhausted = ASC->HasMatchingGameplayTag(TAG_Arcane_Exhausted);
	const bool bNowExhausted = EvaluateExhaustion(GetMana(), GetMaxMana(), RecoveryFraction, bWasExhausted);
	if (bNowExhausted == bWasExhausted)
	{
		return;
	}

	FGameplayTagContainer ExhaustedTag;
	ExhaustedTag.AddTag(TAG_Arcane_Exhausted);

	if (bNowExhausted)
	{
		ASC->AddLooseGameplayTags(ExhaustedTag);
		ASC->ExecuteGameplayCue(TAG_Cue_Arcane_Exhausted);
	}
	else
	{
		ASC->RemoveLooseGameplayTags(ExhaustedTag);
	}
}
