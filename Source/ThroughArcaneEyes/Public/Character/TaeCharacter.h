// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "TaeCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAbilitySystemComponent;
class UTaeManaAttributeSet;
class UGameplayAbility;

UCLASS()
class THROUGHARCANEEYES_API ATaeCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATaeCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FGameplayAbilitySpecHandle GetSpectralShiftHandle() const { return SpectralShiftHandle; }

protected:
	virtual void BeginPlay() override;

private:
	// Spring arm — close over-the-shoulder offset; rotation follows controller
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	// Follow camera — attached to spring arm end; inherits arm rotation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// Ability class granted on BeginPlay — assign BP_GA_SpectralShift in BP_Hero Class Defaults
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> SpectralShiftAbility;

	FGameplayAbilitySpecHandle SpectralShiftHandle;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UTaeManaAttributeSet> ManaAttributeSet;
};
