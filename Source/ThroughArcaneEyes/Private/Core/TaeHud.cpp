// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/TaeHud.h"
#include "Blueprint/UserWidget.h"

void ATaeHud::BeginPlay()
{
	Super::BeginPlay();

	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UUserWidget>(GetWorld(), MainWidgetClass);
		if (MainWidget)
		{
			MainWidget->AddToViewport();
		}
	}
}
