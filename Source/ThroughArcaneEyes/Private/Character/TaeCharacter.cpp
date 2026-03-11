// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/TaeCharacter.h"
#include "Camera/CameraComponent.h"

ATaeCharacter::ATaeCharacter()
{
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetRootComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 60.f)); // eye height
	FirstPersonCamera->bUsePawnControlRotation = true;
}

void ATaeCharacter::BeginPlay()
{
	Super::BeginPlay();
}
