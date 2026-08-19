// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "GAS/TaeCueNotifies.h"
#include "Core/TaeGameInstance.h"
#include "Core/TaeArcaneSubsystem.h"
#include "UI/TaeHudViewModel.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

namespace
{
	// Cues reach the UI the same way everything else does — never by touching a widget directly
	UTaeHudViewModel* FindViewModel(const AActor* Target)
	{
		const UWorld* World = Target ? Target->GetWorld() : nullptr;
		const UTaeGameInstance* GI = World ? World->GetGameInstance<UTaeGameInstance>() : nullptr;
		return GI ? GI->GetHudViewModel() : nullptr;
	}
}

bool UTaeCueNotify_ManaDrain::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->BeginManaFlow(ETaeManaFlow::Draining);
	}
	return true;
}

bool UTaeCueNotify_ManaDrain::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->EndManaFlow(ETaeManaFlow::Draining);
	}
	return true;
}

bool UTaeCueNotify_ManaRegen::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->BeginManaFlow(ETaeManaFlow::Regenerating);
	}
	return true;
}

bool UTaeCueNotify_ManaRegen::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->EndManaFlow(ETaeManaFlow::Regenerating);
	}
	return true;
}

bool UTaeCueNotify_ArcaneExhausted::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	const UWorld* World = MyTarget ? MyTarget->GetWorld() : nullptr;
	if (UTaeArcaneSubsystem* Arcane = World ? World->GetSubsystem<UTaeArcaneSubsystem>() : nullptr)
	{
		Arcane->FlashVignette();
	}
	return true;
}
