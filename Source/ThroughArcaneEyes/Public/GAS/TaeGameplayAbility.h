// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TaeGameplayAbility.generated.h"

UCLASS(Abstract)
class THROUGHARCANEEYES_API UTaeGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Auto-end after this many seconds; 0 = player must cancel manually
	UPROPERTY(EditDefaultsOnly, Category = "Tae")
	float AbilityDuration = 0.f;

private:
	void OnDurationExpired();

	FTimerHandle DurationHandle;
};
