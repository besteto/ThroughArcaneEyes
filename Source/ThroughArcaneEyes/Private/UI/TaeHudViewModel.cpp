// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "UI/TaeHudViewModel.h"

void UTaeHudViewModel::SetMana(float NewMana)
{
	UE_MVVM_SET_PROPERTY_VALUE(Mana, NewMana);
	UE_MVVM_SET_PROPERTY_VALUE(ManaText, FText::AsNumber(FMath::RoundToInt(NewMana)));
	RefreshManaPercent();
}

void UTaeHudViewModel::SetArcaneActive(bool bNewArcaneActive)
{
	UE_MVVM_SET_PROPERTY_VALUE(bArcaneActive, bNewArcaneActive);
	UE_MVVM_SET_PROPERTY_VALUE(ArcaneVisibility, bNewArcaneActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

ETaeManaFlow UTaeHudViewModel::ResolveManaFlow(const int32 InDrainCount, const int32 InRegenCount)
{
	if (InDrainCount > 0)
	{
		return ETaeManaFlow::Draining;
	}
	if (InRegenCount > 0)
	{
		return ETaeManaFlow::Regenerating;
	}
	return ETaeManaFlow::Idle;
}

void UTaeHudViewModel::SetMaxMana(const float NewMaxMana)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxMana, NewMaxMana);
	RefreshManaPercent();
}

void UTaeHudViewModel::SetExhausted(const bool bNewExhausted)
{
	UE_MVVM_SET_PROPERTY_VALUE(bExhausted, bNewExhausted);
	UE_MVVM_SET_PROPERTY_VALUE(ExhaustedVisibility,
		bNewExhausted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UTaeHudViewModel::BeginManaFlow(const ETaeManaFlow Flow)
{
	if (Flow == ETaeManaFlow::Draining) { ++DrainCount; }
	else if (Flow == ETaeManaFlow::Regenerating) { ++RegenCount; }
	RefreshManaFlow();
}

void UTaeHudViewModel::EndManaFlow(const ETaeManaFlow Flow)
{
	if (Flow == ETaeManaFlow::Draining) { DrainCount = FMath::Max(DrainCount - 1, 0); }
	else if (Flow == ETaeManaFlow::Regenerating) { RegenCount = FMath::Max(RegenCount - 1, 0); }
	RefreshManaFlow();
}

void UTaeHudViewModel::RefreshManaFlow()
{
	const ETaeManaFlow NewFlow = ResolveManaFlow(DrainCount, RegenCount);
	UE_MVVM_SET_PROPERTY_VALUE(ManaFlow, NewFlow);

	FLinearColor NewTint = FLinearColor::White;
	if (NewFlow == ETaeManaFlow::Draining)
	{
		NewTint = FLinearColor(1.f, 0.45f, 0.2f);
	}
	else if (NewFlow == ETaeManaFlow::Regenerating)
	{
		NewTint = FLinearColor(0.4f, 1.f, 0.5f);
	}
	UE_MVVM_SET_PROPERTY_VALUE(ManaBarTint, NewTint);
}

void UTaeHudViewModel::RefreshManaPercent()
{
	const float NewPercent = MaxMana > 0.f ? FMath::Clamp(Mana / MaxMana, 0.f, 1.f) : 0.f;
	UE_MVVM_SET_PROPERTY_VALUE(ManaPercent, NewPercent);
}
