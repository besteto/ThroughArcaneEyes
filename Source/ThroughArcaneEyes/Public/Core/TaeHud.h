// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TaeHud.generated.h"

class UUserWidget;

UCLASS()
class THROUGHARCANEEYES_API ATaeHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	// Assign your main UMG widget blueprint here in BP_HUD
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainWidget;
};
