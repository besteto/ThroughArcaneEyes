// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Character/TaeCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/TaeManaAttributeSet.h"
#include "GAS/GA_SpectralShift.h"

ATaeCharacter::ATaeCharacter()
{
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetRootComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 60.f)); // eye height
	FirstPersonCamera->bUsePawnControlRotation = true;

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmsMesh->SetupAttachment(FirstPersonCamera);
	ArmsMesh->bOnlyOwnerSee = true;
	ArmsMesh->bCastDynamicShadow = false;

	// Hide the full-body mesh from the owning player (arms replace it in first-person)
	GetMesh()->bOwnerNoSee = true;

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
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_SpectralShift::StaticClass()));
}
