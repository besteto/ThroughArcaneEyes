// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/TaeGameMode.h"
// AGameMode pairs with AGameState; AGameModeBase pairs with AGameStateBase
#include "Core/TaeGameState.h"
#include "Core/TaeHud.h"
#include "Character/TaeCharacter.h"
#include "Character/TaePlayerController.h"

ATaeGameMode::ATaeGameMode()
{
	GameStateClass       = ATaeGameState::StaticClass();
	HUDClass             = ATaeHud::StaticClass();
	DefaultPawnClass     = ATaeCharacter::StaticClass();
	PlayerControllerClass = ATaePlayerController::StaticClass();
}
