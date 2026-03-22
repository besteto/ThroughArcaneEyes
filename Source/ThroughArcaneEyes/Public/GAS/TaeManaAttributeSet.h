// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GAS/TaeGASTypes.h"
#include "TaeManaAttributeSet.generated.h"

UCLASS()
class THROUGHARCANEEYES_API UTaeManaAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTaeManaAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UPROPERTY(BlueprintReadOnly, Category = "Tae|Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UTaeManaAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Tae|Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UTaeManaAttributeSet, MaxMana)
};
