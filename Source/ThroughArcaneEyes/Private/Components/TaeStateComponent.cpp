// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Components/TaeStateComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "ThroughArcaneEyes.h"

UTaeStateComponent::UTaeStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAbilitySystemComponent* UTaeStateComponent::FindRelevantASC(AActor* Owner)
{
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}

	if (const UWorld* World = Owner ? Owner->GetWorld() : nullptr)
	{
		if (const APlayerController* PC = World->GetFirstPlayerController())
		{
			if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PC->GetPawn()))
			{
				return ASI->GetAbilitySystemComponent();
			}
		}
	}

	return nullptr;
}

void UTaeStateComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAbilitySystemComponent* ASC = FindRelevantASC(GetOwner()))
	{
		const FGameplayTag ArcaneTag = FGameplayTag::RequestGameplayTag(FName("Arcane.Vision"));
		ASC->RegisterGameplayTagEvent(ArcaneTag, EGameplayTagEventType::AnyCountChange)
			.AddUObject(this, &UTaeStateComponent::OnArcaneTagChanged);
	}
	else
	{
		UE_LOG(LogTae, Warning, TEXT("[StateComponent] No ASC found for %s — Arcane events will not fire"), *GetOwner()->GetName());
	}
}

void UTaeStateComponent::OnArcaneTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	OnArcaneStateChanged.Broadcast(NewCount > 0);
}
