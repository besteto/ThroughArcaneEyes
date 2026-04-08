// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TaeActivatableWidget.generated.h"

// Base class for all Tae activatable widgets.
// Automatically collapses on deactivation and restores visibility on activation.
// Reparent WBP_PauseMenu, WBP_WinScreen, WBP_MainMenu to this class.
UCLASS()
class THROUGHARCANEEYES_API UTaeActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
};
