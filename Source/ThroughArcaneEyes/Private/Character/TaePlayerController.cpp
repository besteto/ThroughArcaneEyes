// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Character/TaePlayerController.h"
#include "ThroughArcaneEyes.h"
#include "Character/TaeCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaAttributeSet.h"
#include "Core/TaeGameInstance.h"
#include "UI/TaeHudViewModel.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

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
		EiComp->BindAction(MoveAction,          ETriggerEvent::Triggered, this, &ThisClass::DoMove);
		EiComp->BindAction(LookAction,          ETriggerEvent::Triggered, this, &ThisClass::DoLook);
		EiComp->BindAction(JumpAction,          ETriggerEvent::Started,   this, &ThisClass::DoJump);
		EiComp->BindAction(JumpAction,          ETriggerEvent::Completed, this, &ThisClass::DoStopJumping);
		EiComp->BindAction(SpectralShiftAction, ETriggerEvent::Started, this, &ThisClass::DoSpectralShift);
		EiComp->BindAction(PauseAction,         ETriggerEvent::Started, this, &ThisClass::DoPause);
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
		return;
	}

	if (!OwnerCharacter) return;

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	UTaeGameInstance* GI = GetGameInstance<UTaeGameInstance>();
	if (!ASC || !GI) return;

	UTaeHudViewModel* VM = GI->GetHudViewModel();
	if (!VM) return;

	// Arcane tag → bArcaneActive
	ASC->RegisterGameplayTagEvent(TAG_Arcane_Vision, EGameplayTagEventType::AnyCountChange)
		.AddWeakLambda(VM, [VM](const FGameplayTag&, int32 Count)
		{
			VM->SetArcaneActive(Count > 0);
		});

	// Mana attribute → Mana
	ASC->GetGameplayAttributeValueChangeDelegate(UTaeManaAttributeSet::GetManaAttribute())
		.AddWeakLambda(VM, [VM](const FOnAttributeChangeData& Data)
		{
			VM->SetMana(Data.NewValue);
		});

	// Push current values immediately
	VM->SetArcaneActive(ASC->HasMatchingGameplayTag(TAG_Arcane_Vision));
	VM->SetMana(ASC->GetNumericAttribute(UTaeManaAttributeSet::GetManaAttribute()));
}

void ATaePlayerController::DoMove(const FInputActionInstance& Action)
{
	if (!OwnerCharacter) return;
	const FVector2D Axis = Action.GetValue().Get<FVector2D>();
	OwnerCharacter->AddMovementInput(OwnerCharacter->GetActorForwardVector(), Axis.Y);
	OwnerCharacter->AddMovementInput(OwnerCharacter->GetActorRightVector(),   Axis.X);
}

void ATaePlayerController::DoJump(const FInputActionInstance& Action)
{
	if (OwnerCharacter) OwnerCharacter->Jump();
}

void ATaePlayerController::DoStopJumping(const FInputActionInstance& Action)
{
	if (OwnerCharacter) OwnerCharacter->StopJumping();
}

void ATaePlayerController::DoLook(const FInputActionInstance& Action)
{
	const FVector2D Axis = Action.GetValue().Get<FVector2D>();
	AddYawInput(Axis.X);
	AddPitchInput(Axis.Y);
}

void ATaePlayerController::DoPause(const FInputActionInstance& Action)
{
	OnPauseRequested();
}

void ATaePlayerController::DoSpectralShift(const FInputActionInstance& Action)
{
	if (!OwnerCharacter) return;

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;

	const FGameplayAbilitySpecHandle Handle = OwnerCharacter->GetSpectralShiftHandle();
	if (ASC->HasMatchingGameplayTag(TAG_Arcane_Vision))
	{
		ASC->CancelAbilityHandle(Handle);
	}
	else
	{
		ASC->TryActivateAbility(Handle);
	}
}

#if WITH_EDITOR
EDataValidationResult ATaePlayerController::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!DefaultMappingContext)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoMappingContext", "TaePlayerController: DefaultMappingContext is not assigned."));
		Result = EDataValidationResult::Invalid;
	}
	if (!MoveAction)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoMoveAction", "TaePlayerController: MoveAction is not assigned."));
		Result = EDataValidationResult::Invalid;
	}
	if (!LookAction)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoLookAction", "TaePlayerController: LookAction is not assigned."));
		Result = EDataValidationResult::Invalid;
	}
	if (!JumpAction)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoJumpAction", "TaePlayerController: JumpAction is not assigned."));
		Result = EDataValidationResult::Invalid;
	}
	if (!SpectralShiftAction)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoSpectralShiftAction", "TaePlayerController: SpectralShiftAction is not assigned."));
		Result = EDataValidationResult::Invalid;
	}
	if (!PauseAction)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoPauseAction", "TaePlayerController: PauseAction is not assigned."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
