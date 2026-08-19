// Copyright © 2026 Helen Allien Poe. See LICENSE.

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
