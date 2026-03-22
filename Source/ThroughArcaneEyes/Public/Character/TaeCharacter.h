// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TaeCharacter.generated.h"

class UCameraComponent;

UCLASS()
class THROUGHARCANEEYES_API ATaeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATaeCharacter();

protected:
	virtual void BeginPlay() override;

private:
	// First-person camera — attach to root, positioned at eye level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
};
