// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Character/TaeCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/TaeManaAttributeSet.h"
#include "Abilities/GameplayAbility.h"

ATaeCharacter::ATaeCharacter()
{
	// Close over-the-shoulder spring arm — short length, right-shoulder offset
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 80.f;
	SpringArm->SocketOffset = FVector(0.f, 50.f, 20.f); // right of spine, slightly up
	SpringArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // inherits from arm

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ManaAttributeSet = CreateDefaultSubobject<UTaeManaAttributeSet>(TEXT("ManaAttributeSet"));
}

UAbilitySystemComponent* ATaeCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATaeCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	if (SpectralShiftAbility)
	{
		SpectralShiftHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SpectralShiftAbility));
	}
}
