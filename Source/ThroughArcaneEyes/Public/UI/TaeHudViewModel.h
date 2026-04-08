// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Components/SlateWrapperTypes.h"
#include "TaeHudViewModel.generated.h"

UCLASS()
class THROUGHARCANEEYES_API UTaeHudViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetMana(float NewMana);
	void SetArcaneActive(bool bNewArcaneActive);

	// Raw values — for logic use
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	float Mana = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	bool bArcaneActive = false;

	// Presentation values — bind these directly in View Bindings, no converter needed
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	FText ManaText = FText::FromString(TEXT("0"));

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	ESlateVisibility ArcaneVisibility = ESlateVisibility::Collapsed;
};
