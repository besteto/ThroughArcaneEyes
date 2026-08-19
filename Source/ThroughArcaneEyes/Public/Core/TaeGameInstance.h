// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TaeGameInstance.generated.h"

class UTaeHudViewModel;
class USoundBase;

UCLASS()
class THROUGHARCANEEYES_API UTaeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	// Identifier WBP_HUD uses to fetch the HUD viewmodel out of the global collection. Must match the
	// "Global Viewmodel Identifier" set in the widget's Viewmodels panel, or the widget silently falls
	// back to creating its own instance that nothing writes to.
	static const FName HudViewModelContextName;

	UFUNCTION(BlueprintCallable)
	UTaeHudViewModel* GetHudViewModel() const { return HudViewModel; }

	USoundBase* GetForestMusic() const { return Music_Forest; }
	USoundBase* GetArcaneMusic() const { return Music_Arcane; }
	float GetMusicCrossfadeDuration() const { return MusicCrossfadeDuration; }

protected:
	// Assign Music_Forest in BP_TaeGameInstance
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Audio")
	TObjectPtr<USoundBase> Music_Forest;

	// Assign Music_Arcane in BP_TaeGameInstance
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Audio")
	TObjectPtr<USoundBase> Music_Arcane;

	UPROPERTY(EditDefaultsOnly, Category = "Tae|Audio")
	float MusicCrossfadeDuration = 1.5f;

private:
	UPROPERTY()
	TObjectPtr<UTaeHudViewModel> HudViewModel;
};
