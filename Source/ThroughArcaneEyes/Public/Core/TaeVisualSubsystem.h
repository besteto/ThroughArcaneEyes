// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TaeVisualSubsystem.generated.h"

class APostProcessVolume;
class UAudioComponent;
class USoundBase;

UCLASS()
class THROUGHARCANEEYES_API UTaeVisualSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Called by GA_SpectralShift Activate/End — toggles volume + crossfades music
	void SetArcaneActive(bool bActive);

	// Spike vignette intensity then fade back; Duration is total fade-out time
	void FlashVignette(float Duration = 0.5f);

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void CrossfadeMusic(bool bToArcane);
	void ClearVignetteFlash();

	UPROPERTY()
	TObjectPtr<APostProcessVolume> SpectralVolume;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ForestMusicComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ArcaneMusicComp;

	FTimerHandle VignetteTimerHandle;
	bool bVolumeActive = false;
};
