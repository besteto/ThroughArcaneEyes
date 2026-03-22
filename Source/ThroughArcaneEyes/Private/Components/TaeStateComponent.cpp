// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Components/TaeStateComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

UTaeStateComponent::UTaeStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTaeStateComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const FGameplayTag ArcaneTag = FGameplayTag::RequestGameplayTag(FName("Arcane.Vision"));
			ASC->RegisterGameplayTagEvent(ArcaneTag, EGameplayTagEventType::AnyCountChange)
				.AddUObject(this, &UTaeStateComponent::OnArcaneTagChanged);
		}
	}
}

void UTaeStateComponent::OnArcaneTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	OnArcaneStateChanged.Broadcast(NewCount > 0);
}
