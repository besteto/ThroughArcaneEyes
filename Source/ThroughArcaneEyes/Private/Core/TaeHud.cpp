// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Core/TaeHud.h"
#include "UI/TaeMainMenuWidget.h"
#include "UI/TaeActivatableWidget.h"
#include "Blueprint/UserWidget.h"

void ATaeHud::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuClass)
	{
		MainMenu = CreateWidget<UTaeMainMenuWidget>(GetOwningPlayerController(), MainMenuClass);
		if (MainMenu)
		{
			MainMenu->AddToPlayerScreen(1);
			MainMenu->ActivateWidget();
		}
	}

	if (HudWidgetClass)
	{
		HudWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), HudWidgetClass);
		if (HudWidget)
		{
			HudWidget->AddToPlayerScreen(0);
		}
	}

}


void ATaeHud::ShowMainMenu()
{
	if (VictoryScreen) VictoryScreen->DeactivateWidget();
	if (PauseMenu) PauseMenu->DeactivateWidget();
	if (MainMenu) MainMenu->ActivateWidget();
}

void ATaeHud::TogglePauseMenu()
{
	if (!PauseMenuClass) return;

	if (!PauseMenu)
	{
		PauseMenu = CreateWidget<UTaeActivatableWidget>(GetOwningPlayerController(), PauseMenuClass);
		PauseMenu->AddToPlayerScreen(2);
	}

	if (PauseMenu->IsActivated())
	{
		PauseMenu->DeactivateWidget();
	}
	else
	{
		PauseMenu->ActivateWidget();
	}
}

void ATaeHud::ShowVictoryScreen()
{
	if (!VictoryScreenClass) return;

	if (!VictoryScreen)
	{
		VictoryScreen = CreateWidget<UTaeActivatableWidget>(GetOwningPlayerController(), VictoryScreenClass);
		VictoryScreen->AddToPlayerScreen(2);
	}

	if (!VictoryScreen->IsActivated())
	{
		VictoryScreen->ActivateWidget();
	}
}
