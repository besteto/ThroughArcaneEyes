// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TaeHud.generated.h"

class UTaeMainMenuWidget;

UCLASS()
class THROUGHARCANEEYES_API ATaeHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	// Assign WBP_MainMenu in BP_TaeHud (Details > UI)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTaeMainMenuWidget> MainMenuClass;

	UPROPERTY()
	TObjectPtr<UTaeMainMenuWidget> MainMenu;
};
