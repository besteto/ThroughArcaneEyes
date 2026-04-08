// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "UI/TaeActivatableWidget.h"

void UTaeActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetVisibility(ESlateVisibility::Visible);
}

void UTaeActivatableWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	SetVisibility(ESlateVisibility::Collapsed);
}
