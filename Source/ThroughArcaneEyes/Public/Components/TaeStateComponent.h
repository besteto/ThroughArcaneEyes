// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "TaeStateComponent.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArcaneStateChanged, bool, bArcaneActive);

UCLASS(ClassGroup = "Tae", meta = (BlueprintSpawnableComponent))
class THROUGHARCANEEYES_API UTaeStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTaeStateComponent();

	UPROPERTY(BlueprintAssignable, Category = "Tae")
	FOnArcaneStateChanged OnArcaneStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	// Tries the owner first; falls back to the local player's pawn. Returns null if neither has an ASC.
	static UAbilitySystemComponent* FindRelevantASC(AActor* Owner);

	void OnArcaneTagChanged(const FGameplayTag Tag, int32 NewCount);
};
