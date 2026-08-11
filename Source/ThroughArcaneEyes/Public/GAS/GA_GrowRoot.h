// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GAS/TaeGameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "GA_GrowRoot.generated.h"

class ATaeRootAnchor;
class ATaeRootPath;
class UGameplayEffect;

// Hold-to-channel root growth. Activates only while Arcane.Vision is active and the avatar overlaps an
// ATaeRootAnchor. Drains mana per second and advances that anchor's path. Growth is permanent — ending
// early leaves the path partially grown.
UCLASS()
class THROUGHARCANEEYES_API UGA_GrowRoot : public UTaeGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_GrowRoot();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	// Finds the nearest overlapping anchor on the avatar, or nullptr
	ATaeRootAnchor* FindAnchorInRange(const FGameplayAbilityActorInfo* ActorInfo) const;

	void TickGrowth();

	// Alpha added per second of channelling — a full path takes 1/GrowthRate seconds
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot", meta = (ClampMin = "0.001"))
	float GrowthRate = 0.35f;

	// Mana drained per second while channelling
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot", meta = (ClampMin = "0.0"))
	float ManaCostPerSecond = 12.f;

	// How often growth is applied. Coarser than frame rate; growth is not frame-dependent.
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot", meta = (ClampMin = "0.01"))
	float GrowthTickInterval = 0.05f;

	UPROPERTY()
	TObjectPtr<ATaeRootPath> ActivePath;

	// Defaults to UTaeManaDrainEffect; overridable in BP_GA_GrowRoot
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot")
	TSubclassOf<UGameplayEffect> DrainEffectClass;

	FTimerHandle GrowthTimerHandle;

	FActiveGameplayEffectHandle DrainHandle;

	void OnArcaneVisionChanged(const FGameplayTag Tag, int32 NewCount);

	FDelegateHandle ArcaneVisionHandle;
};
