// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "GAS/TaeManaEffects.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaAttributeSet.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UTaeManaEffectBase::UTaeManaEffectBase()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(PeriodSeconds);

	// The first tick lands one period in, so applying and immediately removing costs nothing
	bExecutePeriodicEffectOnApplication = false;

	FSetByCallerFloat RateMagnitude;
	RateMagnitude.DataTag = TAG_Data_ManaRate;

	FGameplayModifierInfo ManaModifier;
	ManaModifier.Attribute = UTaeManaAttributeSet::GetManaAttribute();
	// AddBase, not the hidden backwards-compat Additive alias
	ManaModifier.ModifierOp = EGameplayModOp::AddBase;
	ManaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(RateMagnitude);
	Modifiers.Add(ManaModifier);
}

UTaeManaDrainEffect::UTaeManaDrainEffect()
{
	FGameplayEffectCue DrainCue;
	DrainCue.GameplayCueTags.AddTag(TAG_Cue_Mana_Drain);
	GameplayCues.Add(DrainCue);
}

UTaeManaRegenEffect::UTaeManaRegenEffect()
{
	FGameplayEffectCue RegenCue;
	RegenCue.GameplayCueTags.AddTag(TAG_Cue_Mana_Regen);
	GameplayCues.Add(RegenCue);

	// Standing in a grove does not refill you mid-survey. Inhibition keeps the effect applied, so
	// leaving Arcane resumes regen without re-entering the volume.
	//
	// UGameplayEffect::OngoingTagRequirements is deprecated in 5.8, so the requirement lives on a
	// component. It must be created with CreateDefaultSubobject rather than FindOrAddComponent:
	// FindOrAddComponent calls NewObject with an empty name, which is fatal inside a constructor.
	// UGameplayEffect::PostInitProperties expects native components to be made exactly this way.
	UTargetTagRequirementsGameplayEffectComponent* TagRequirements =
		CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>(TEXT("TargetTagRequirements"));
	TagRequirements->OngoingTagRequirements.IgnoreTags.AddTag(TAG_Arcane_Vision);
	GEComponents.Add(TagRequirements);
}
