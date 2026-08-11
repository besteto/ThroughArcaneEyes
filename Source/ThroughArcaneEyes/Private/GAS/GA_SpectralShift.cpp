// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/GA_SpectralShift.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaEffects.h"
#include "Core/TaeArcaneSubsystem.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"

UGA_SpectralShift::UGA_SpectralShift()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	DrainEffectClass = UTaeManaDrainEffect::StaticClass();
}

void UGA_SpectralShift::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Grant Arcane.Vision tag
	FGameplayTagContainer Tags;
	Tags.AddTag(TAG_Arcane_Vision);
	ActorInfo->AbilitySystemComponent->AddLooseGameplayTags(Tags);

	// Push Arcane input context
	if (ArcaneInputContext)
	{
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			if (auto* EiSys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				EiSys->AddMappingContext(ArcaneInputContext, 1);
			}
		}
	}

	if (UTaeArcaneSubsystem* Visual = GetWorld()->GetSubsystem<UTaeArcaneSubsystem>())
	{
		Visual->SetArcaneActive(true);
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC && DrainEffectClass)
	{
		const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DrainEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			// Negative — this is a drain. Per-period, not per-second.
			Spec.Data->SetSetByCallerMagnitude(
				TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(-ArcaneDrainPerSecond));
			DrainHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}
}

void UGA_SpectralShift::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove Arcane.Vision tag
	FGameplayTagContainer Tags;
	Tags.AddTag(TAG_Arcane_Vision);
	ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTags(Tags);

	// Pop Arcane input context
	if (ArcaneInputContext)
	{
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			if (auto* EiSys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				EiSys->RemoveMappingContext(ArcaneInputContext);
			}
		}
	}

	if (UTaeArcaneSubsystem* Visual = GetWorld()->GetSubsystem<UTaeArcaneSubsystem>())
	{
		Visual->SetArcaneActive(false);
	}

	if (DrainHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			ASC->RemoveActiveGameplayEffect(DrainHandle);
		}
		DrainHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
