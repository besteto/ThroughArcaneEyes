// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/GA_GrowRoot.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaEffects.h"
#include "World/TaeRootAnchor.h"
#include "World/TaeRootPath.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "ThroughArcaneEyes.h"

UGA_GrowRoot::UGA_GrowRoot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(TAG_Arcane_Growing);
	DrainEffectClass = UTaeManaDrainEffect::StaticClass();
}

bool UGA_GrowRoot::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Growing is an Arcane-mode act
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC || !ASC->HasMatchingGameplayTag(TAG_Arcane_Vision))
	{
		return false;
	}

	return FindAnchorInRange(ActorInfo) != nullptr;
}

ATaeRootAnchor* UGA_GrowRoot::FindAnchorInRange(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!Avatar)
	{
		return nullptr;
	}

	TArray<AActor*> Overlapping;
	Avatar->GetOverlappingActors(Overlapping, ATaeRootAnchor::StaticClass());

	ATaeRootAnchor* Nearest = nullptr;
	float NearestDistSq = TNumericLimits<float>::Max();

	for (AActor* Actor : Overlapping)
	{
		ATaeRootAnchor* Anchor = Cast<ATaeRootAnchor>(Actor);
		if (!Anchor || !Anchor->GetPath())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Avatar->GetActorLocation(), Anchor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Anchor;
		}
	}

	return Nearest;
}

void UGA_GrowRoot::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ATaeRootAnchor* Anchor = FindAnchorInRange(ActorInfo);
	if (!Anchor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActivePath = Anchor->GetPath();

	UWorld* World = GetWorld();
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	World->GetTimerManager().SetTimer(
		GrowthTimerHandle, this, &UGA_GrowRoot::TickGrowth, GrowthTickInterval, true);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (DrainEffectClass)
		{
			const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DrainEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				// Stacks with the vision drain already applied by UGA_SpectralShift
				Spec.Data->SetSetByCallerMagnitude(
					TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(-ManaCostPerSecond));
				DrainHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}

		ArcaneVisionHandle = ASC->RegisterGameplayTagEvent(TAG_Arcane_Vision, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UGA_GrowRoot::OnArcaneVisionChanged);
	}
}

void UGA_GrowRoot::OnArcaneVisionChanged(const FGameplayTag Tag, const int32 NewCount)
{
	// Growing is an Arcane-mode act — losing the vision cancels the channel, keeping partial growth
	if (NewCount <= 0)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UGA_GrowRoot::TickGrowth()
{
	if (!ActivePath)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ActivePath->AdvanceGrowth(GrowthRate * GrowthTickInterval);

	// Finished — stop rather than burning mana on a full path
	if (ActivePath->GetConnectionState() == ETaeConnectionState::Restored)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_GrowRoot::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GrowthTimerHandle);
	}
	ActivePath = nullptr;

	if (DrainHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(DrainHandle);
		}
		DrainHandle.Invalidate();
	}

	if (ArcaneVisionHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RegisterGameplayTagEvent(TAG_Arcane_Vision, EGameplayTagEventType::NewOrRemoved).Remove(ArcaneVisionHandle);
		}
		ArcaneVisionHandle.Reset();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
