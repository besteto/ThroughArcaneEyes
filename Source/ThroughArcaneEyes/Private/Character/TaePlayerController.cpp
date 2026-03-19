// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/TaePlayerController.h"
#include "ThroughArcaneEyes.h"
#include "Character/TaeCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ATaePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!DefaultMappingContext)
	{
		UE_LOG(LogTae, Warning, TEXT("[PC] DefaultMappingContext is NULL — assign it in BP_TaePlayerController (Details > Tae)"));
	}

	if (const auto EiSys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		EiSys->AddMappingContext(DefaultMappingContext, 0);
	}
	else
	{
		UE_LOG(LogTae, Warning, TEXT("[PC] EnhancedInputLocalPlayerSubsystem not found — no local player yet?"));
	}
}

void ATaePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EiComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (!MoveAction)        UE_LOG(LogTae, Warning, TEXT("[PC] MoveAction is NULL — assign it in BP_TaePlayerController"));
		if (!LookAction)        UE_LOG(LogTae, Warning, TEXT("[PC] LookAction is NULL — assign it in BP_TaePlayerController"));
		if (!JumpAction)        UE_LOG(LogTae, Warning, TEXT("[PC] JumpAction is NULL — assign it in BP_TaePlayerController"));
		if (!ToggleEyesAction)  UE_LOG(LogTae, Warning, TEXT("[PC] ToggleEyesAction is NULL — assign it in BP_TaePlayerController"));

		EiComp->BindAction(MoveAction,        ETriggerEvent::Triggered, this, &ThisClass::DoMove);
		EiComp->BindAction(LookAction,        ETriggerEvent::Triggered, this, &ThisClass::DoLook);
		EiComp->BindAction(JumpAction,        ETriggerEvent::Started,   this, &ThisClass::DoJump);
		EiComp->BindAction(JumpAction,        ETriggerEvent::Completed, this, &ThisClass::DoStopJumping);
		EiComp->BindAction(ToggleEyesAction,  ETriggerEvent::Started,   this, &ThisClass::DoToggleEyes);
	}
	else
	{
		UE_LOG(LogTae, Error, TEXT("[PC] InputComponent is NOT an EnhancedInputComponent (got: %s) — check DefaultInput.ini"),
			InputComponent ? *InputComponent->GetClass()->GetName() : TEXT("NULL"));
	}
}

void ATaePlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	OwnerCharacter = Cast<ATaeCharacter>(InPawn);

	if (InPawn && !OwnerCharacter)
	{
		UE_LOG(LogTae, Warning, TEXT("[PC] SetPawn received a pawn that is not ATaeCharacter: %s"), *InPawn->GetClass()->GetName());
	}
}

void ATaePlayerController::DoMove(IA_t Action)
{
	if (!OwnerCharacter) return;
	const FVector2D Axis = Action.GetValue().Get<FVector2D>();
	OwnerCharacter->AddMovementInput(OwnerCharacter->GetActorForwardVector(), Axis.Y);
	OwnerCharacter->AddMovementInput(OwnerCharacter->GetActorRightVector(),   Axis.X);
}

void ATaePlayerController::DoJump(IA_t Action)
{
	if (OwnerCharacter) OwnerCharacter->Jump();
}

void ATaePlayerController::DoStopJumping(IA_t Action)
{
	if (OwnerCharacter) OwnerCharacter->StopJumping();
}

void ATaePlayerController::DoLook(IA_t Action)
{
	const FVector2D Axis = Action.GetValue().Get<FVector2D>();
	AddYawInput(Axis.X);
	AddPitchInput(Axis.Y);
}

void ATaePlayerController::DoToggleEyes(IA_t Action)
{
}
