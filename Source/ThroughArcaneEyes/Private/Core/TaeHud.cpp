// Copyright Epic Games, Inc. All Rights Reserved.

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
