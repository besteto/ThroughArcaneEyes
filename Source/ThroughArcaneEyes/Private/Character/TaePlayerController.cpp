// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/TaePlayerController.h"
#include "Character/TaeCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ATaePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (const auto EiSys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		EiSys->AddMappingContext(DefaultMappingContext, 0);
	}
}

void ATaePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EiComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EiComp->BindAction(MoveAction,  ETriggerEvent::Triggered, this, &ThisClass::DoMove);
		EiComp->BindAction(LookAction,  ETriggerEvent::Triggered, this, &ThisClass::DoLook);
		EiComp->BindAction(JumpAction,  ETriggerEvent::Started,   this, &ThisClass::DoJump);
		EiComp->BindAction(JumpAction,  ETriggerEvent::Completed, this, &ThisClass::DoStopJumping);
	}
}

void ATaePlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	OwnerCharacter = Cast<ATaeCharacter>(InPawn);
}

void ATaePlayerController::DoMove(IA_t Action)
{
	if (!OwnerCharacter) return;
	const FVector2D Axis = Action.GetValue().Get<FVector2D>();
	OwnerCharacter->AddMovementInput(OwnerCharacter->GetActorForwardVector(), Axis.Y);
	OwnerCharacter->AddMovementInput(OwnerCharacter->GetActorRightVector(),   Axis.X);
}

void ATaePlayerController::DoLook(IA_t Action)
{
	const FVector2D Axis = Action.GetValue().Get<FVector2D>();
	AddYawInput(Axis.X);
	AddPitchInput(Axis.Y);
}

void ATaePlayerController::DoJump(IA_t Action)
{
	if (OwnerCharacter) OwnerCharacter->Jump();
}

void ATaePlayerController::DoStopJumping(IA_t Action)
{
	if (OwnerCharacter) OwnerCharacter->StopJumping();
}
