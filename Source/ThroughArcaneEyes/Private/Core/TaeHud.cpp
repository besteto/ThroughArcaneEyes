// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Core/TaeHud.h"
#include "UI/TaeMainMenuWidget.h"

void ATaeHud::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuClass)
	{
		MainMenu = CreateWidget<UTaeMainMenuWidget>(GetOwningPlayerController(), MainMenuClass);
		if (MainMenu)
		{
			MainMenu->AddToPlayerScreen();
			MainMenu->ActivateWidget();
		}
	}
}
