// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

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
