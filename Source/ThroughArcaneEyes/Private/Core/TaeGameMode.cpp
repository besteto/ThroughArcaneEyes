// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/TaeGameMode.h"
#include "Core/TaeGameState.h"
#include "Core/TaeHud.h"
#include "Character/TaeCharacter.h"

ATaeGameMode::ATaeGameMode()
{
	GameStateClass   = ATaeGameState::StaticClass();
	HUDClass         = ATaeHud::StaticClass();
	DefaultPawnClass = ATaeCharacter::StaticClass();
}
