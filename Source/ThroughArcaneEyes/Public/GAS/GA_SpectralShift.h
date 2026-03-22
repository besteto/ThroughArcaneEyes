// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SpectralShift.generated.h"

class UInputMappingContext;

UCLASS()
class THROUGHARCANEEYES_API UGA_SpectralShift : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SpectralShift();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Assign IMC_Arcane in BP_GA_SpectralShift
	UPROPERTY(EditDefaultsOnly, Category = "Tae")
	TObjectPtr<UInputMappingContext> ArcaneInputContext;
};
