// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "TaeCueNotifies.generated.h"

// Mana cue notifies.
//
// These classes hold the logic; the Blueprint assets that derive from them exist only so the cue manager
// can find them — it builds its registry from an asset registry scan, so a pure C++ class is never
// registered. The asset NAME derives the tag (GC_Mana_Drain -> GameplayCue.Mana.Drain), so GameplayCueTag
// is deliberately left unset here. See the M2 spec §6.
//
// Every override is const: notifies are stateless. Counting lives in UTaeHudViewModel.

UCLASS()
class THROUGHARCANEEYES_API UTaeCueNotify_ManaDrain : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

UCLASS()
class THROUGHARCANEEYES_API UTaeCueNotify_ManaRegen : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

UCLASS()
class THROUGHARCANEEYES_API UTaeCueNotify_ArcaneExhausted : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
