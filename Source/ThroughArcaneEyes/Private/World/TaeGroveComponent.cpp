// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "World/TaeGroveComponent.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaEffects.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Curves/CurveFloat.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ThroughArcaneEyes.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

namespace
{
	// Unreal units are centimetres; curves are authored in metres
	constexpr float UuPerMetre = 100.f;
}

UTaeGroveComponent::UTaeGroveComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Overlap);
	SetGenerateOverlapEvents(true);
	RegenEffectClass = UTaeManaRegenEffect::StaticClass();
}

float UTaeGroveComponent::GetFootprintAreaSqM() const
{
	// Scaled extent is a half-size, so the full footprint is twice each axis
	const FVector Extent = GetScaledBoxExtent();
	const float WidthM = (Extent.X * 2.f) / UuPerMetre;
	const float DepthM = (Extent.Y * 2.f) / UuPerMetre;
	return FMath::Max(WidthM * DepthM, 0.f);
}

float UTaeGroveComponent::RegenRateForArea(const float AreaSqM, const UCurveFloat* Curve)
{
	if (!Curve)
	{
		return 0.f;
	}

	return FMath::Max(Curve->GetFloatValue(AreaSqM), 0.f);
}

float UTaeGroveComponent::GetRegenPerSecond() const
{
	return RegenRateForArea(GetFootprintAreaSqM(), RegenCurve);
}

void UTaeGroveComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UTaeGroveComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UTaeGroveComponent::HandleEndOverlap);

	if (BloomSystem)
	{
		// SpawnSystemAttached is UnsafeDuringActorConstruction, so this cannot move to the constructor
		BloomComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BloomSystem, this, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, /*bAutoDestroy=*/false, /*bAutoActivate=*/true);
	}

	if (BloomComponent)
	{
		// The system sizes and scales itself to the grove it is actually on, so resizing the box in
		// BP_Grove needs no second authored value
		BloomComponent->SetVariableVec3(TaeGroveParams::Extent, GetScaledBoxExtent());
		BloomComponent->SetVariableFloat(TaeGroveParams::RegenPerSecond, GetRegenPerSecond());
		BloomComponent->SetVariableBool(TaeGroveParams::IsOccupied, false);
	}
}

void UTaeGroveComponent::RefreshOccupancy()
{
	if (BloomComponent)
	{
		BloomComponent->SetVariableBool(TaeGroveParams::IsOccupied, ActiveRegen.Num() > 0);
	}
}

void UTaeGroveComponent::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	const IAbilitySystemInterface* AsInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!AsInterface || ActiveRegen.Contains(OtherActor))
	{
		return;
	}

	UAbilitySystemComponent* ASC = AsInterface->GetAbilitySystemComponent();
	if (!ASC || !RegenEffectClass)
	{
		return;
	}

	const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(RegenEffectClass, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	// Positive — this restores. Per-period, not per-second.
	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(GetRegenPerSecond()));
	ActiveRegen.Add(OtherActor, ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data));
	RefreshOccupancy();
}

void UTaeGroveComponent::HandleEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	FActiveGameplayEffectHandle Handle;
	if (!ActiveRegen.RemoveAndCopyValue(OtherActor, Handle) || !Handle.IsValid())
	{
		return;
	}

	const IAbilitySystemInterface* AsInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (UAbilitySystemComponent* ASC = AsInterface ? AsInterface->GetAbilitySystemComponent() : nullptr)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}
	RefreshOccupancy();
}

#if WITH_EDITOR
EDataValidationResult UTaeGroveComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!RegenCurve)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoRegenCurve",
			"TaeGroveComponent: RegenCurve is not assigned — this grove grants no mana."));
		Result = EDataValidationResult::Invalid;
	}

	if (!BloomSystem)
	{
		Context.AddWarning(NSLOCTEXT("TaeValidation", "NoBloomSystem",
			"TaeGroveComponent: BloomSystem is not assigned — this grove is invisible."));
	}

	return Result;
}
#endif
