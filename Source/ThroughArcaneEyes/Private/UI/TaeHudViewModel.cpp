// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "UI/TaeHudViewModel.h"

void UTaeHudViewModel::SetMana(float NewMana)
{
	UE_MVVM_SET_PROPERTY_VALUE(Mana, NewMana);
	UE_MVVM_SET_PROPERTY_VALUE(ManaText, FText::AsNumber(FMath::RoundToInt(NewMana)));
}

void UTaeHudViewModel::SetArcaneActive(bool bNewArcaneActive)
{
	UE_MVVM_SET_PROPERTY_VALUE(bArcaneActive, bNewArcaneActive);	
	UE_MVVM_SET_PROPERTY_VALUE(ArcaneVisibility, bNewArcaneActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
