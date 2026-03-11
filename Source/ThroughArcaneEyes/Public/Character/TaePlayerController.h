// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TaePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class THROUGHARCANEEYES_API ATaePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	using IA_t = const struct FInputActionInstance&;

protected:
	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> JumpAction;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void SetPawn(APawn* InPawn) override;

	void DoMove(IA_t Action);
	void DoLook(IA_t Action);
	void DoJump(IA_t Action);
	void DoStopJumping(IA_t Action);

private:
	UPROPERTY(Transient)
	TObjectPtr<class ATaeCharacter> OwnerCharacter;
};
