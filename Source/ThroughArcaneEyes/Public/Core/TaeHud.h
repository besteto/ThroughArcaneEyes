// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TaeHud.generated.h"

class UTaeMainMenuWidget;
class UTaeActivatableWidget;
class UUserWidget;

UCLASS()
class THROUGHARCANEEYES_API ATaeHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowVictoryScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();

protected:
	// Assign in BP_TaeHud
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTaeMainMenuWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HudWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTaeActivatableWidget> PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTaeActivatableWidget> VictoryScreenClass;

private:
	UPROPERTY()
	TObjectPtr<UTaeMainMenuWidget> MainMenu;

	UPROPERTY()
	TObjectPtr<UUserWidget> HudWidget;

	UPROPERTY()
	TObjectPtr<UTaeActivatableWidget> PauseMenu;

	UPROPERTY()
	TObjectPtr<UTaeActivatableWidget> VictoryScreen;
};
