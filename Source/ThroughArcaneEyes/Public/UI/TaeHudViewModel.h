// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Components/SlateWrapperTypes.h"
#include "TaeHudViewModel.generated.h"

// What mana is doing right now — drives the HUD bar tint
UENUM(BlueprintType)
enum class ETaeManaFlow : uint8
{
	Idle,
	Draining,
	Regenerating
};

UCLASS()
class THROUGHARCANEEYES_API UTaeHudViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetMana(float NewMana);
	void SetArcaneActive(bool bNewArcaneActive);
	void SetMaxMana(float NewMaxMana);
	void SetExhausted(bool bNewExhausted);

	// Called by the mana cue notifies. Counted rather than toggled, because two drains can be active at
	// once (vision plus growth) and the notifies themselves are stateless.
	void BeginManaFlow(ETaeManaFlow Flow);
	void EndManaFlow(ETaeManaFlow Flow);

	// Pure resolution of the two counters into one displayed state. Draining outranks regenerating.
	static ETaeManaFlow ResolveManaFlow(int32 DrainCount, int32 RegenCount);

	// Raw values — for logic use
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	float Mana = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	bool bArcaneActive = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	float MaxMana = 100.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	float ManaPercent = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	bool bExhausted = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	ETaeManaFlow ManaFlow = ETaeManaFlow::Idle;

	// Presentation values — bind these directly in View Bindings, no converter needed
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	FText ManaText = FText::FromString(TEXT("0"));

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	ESlateVisibility ArcaneVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	FLinearColor ManaBarTint = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	ESlateVisibility ExhaustedVisibility = ESlateVisibility::Collapsed;

private:
	void RefreshManaFlow();
	void RefreshManaPercent();

	int32 DrainCount = 0;
	int32 RegenCount = 0;
};
