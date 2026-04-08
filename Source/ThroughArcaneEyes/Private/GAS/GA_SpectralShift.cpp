// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/GA_SpectralShift.h"
#include "GAS/TaeGASTypes.h"
#include "Core/TaeVisualSubsystem.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"

UGA_SpectralShift::UGA_SpectralShift()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
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

	if (UTaeVisualSubsystem* Visual = GetWorld()->GetSubsystem<UTaeVisualSubsystem>())
	{
		Visual->SetArcaneActive(true);
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

	if (UTaeVisualSubsystem* Visual = GetWorld()->GetSubsystem<UTaeVisualSubsystem>())
	{
		Visual->SetArcaneActive(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
