// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TaePlayerController.generated.h"

struct FInputActionInstance;
class UInputMappingContext;
class UInputAction;

UCLASS()
class THROUGHARCANEEYES_API ATaePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

protected:
	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Tae")
	TObjectPtr<UInputAction> SpectralShiftAction;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void SetPawn(APawn* InPawn) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	void DoMove(const FInputActionInstance& Action);
	void DoLook(const FInputActionInstance& Action);
	void DoJump(const FInputActionInstance& Action);
	void DoStopJumping(const FInputActionInstance& Action);

	void DoSpectralShift(const FInputActionInstance& Action);

private:
	UPROPERTY(Transient)
	TObjectPtr<class ATaeCharacter> OwnerCharacter;
};
