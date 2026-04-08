// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Core/TaeGameInstance.h"
#include "UI/TaeHudViewModel.h"

void UTaeGameInstance::Init()
{
	Super::Init();
	HudViewModel = NewObject<UTaeHudViewModel>(this);
}

void UTaeGameInstance::Shutdown()
{
	Super::Shutdown();
}
