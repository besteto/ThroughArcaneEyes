// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GAS/TaeGameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "GA_SpectralShift.generated.h"

class UInputMappingContext;
class UGameplayEffect;

UCLASS()
class THROUGHARCANEEYES_API UGA_SpectralShift : public UTaeGameplayAbility
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

	// Mana drained per second while Arcane Vision is active. Tune in BP_GA_SpectralShift.
	UPROPERTY(EditDefaultsOnly, Category = "Tae", meta = (ClampMin = "0.0"))
	float ArcaneDrainPerSecond = 4.f;

	// Defaults to UTaeManaDrainEffect; overridable in BP_GA_SpectralShift
	UPROPERTY(EditDefaultsOnly, Category = "Tae")
	TSubclassOf<UGameplayEffect> DrainEffectClass;

private:
	void OnExhaustionChanged(const FGameplayTag Tag, int32 NewCount);

	FActiveGameplayEffectHandle DrainHandle;
	FDelegateHandle ExhaustionHandle;

};
