// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TaeArcaneSubsystem.generated.h"

class APostProcessVolume;
class UAudioComponent;
class USoundBase;
class UTaeArcanePalette;
class UMaterialParameterCollection;

UCLASS()
class THROUGHARCANEEYES_API UTaeArcaneSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// Called by GA_SpectralShift Activate/End — sets the blend target and crossfades music
	void SetArcaneActive(bool bActive);

	// Spike vignette intensity then fade back; Duration is total fade-out time
	void FlashVignette(float Duration = 0.5f);

	UFUNCTION(BlueprintPure, Category = "Arcane")
	float GetArcaneBlendAlpha() const { return ArcaneBlendAlpha; }

	// Pure interpolation step — moves Current toward Target so a full traverse takes Duration seconds.
	// Duration <= 0 snaps. Static and world-free so it can be tested directly.
	static float StepBlendAlpha(float Current, float Target, float DeltaTime, float Duration);

	// Copies every palette colour into the collection. Returns how many parameters were written —
	// SetVectorParameterValue returns false for a name the collection does not declare, so a count
	// below the palette's size means the asset and TaeArcaneParams have drifted apart.
	// Static and instance-free so it can be tested against a runtime-built collection.
	static int32 ApplyPaletteToCollection(UWorld* World, const UTaeArcanePalette* Palette, UMaterialParameterCollection* Collection);

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void CrossfadeMusic(bool bToArcane);
	void ApplyBlendAlpha();

	UPROPERTY()
	TObjectPtr<APostProcessVolume> SpectralVolume;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ForestMusicComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ArcaneMusicComp;

	// Seconds for a full Forest <-> Arcane traverse. Post-process, camera, and the M3 overlay all
	// read the resulting alpha so their timings cannot drift apart.
	UPROPERTY(EditAnywhere, Category = "Arcane")
	float ArcaneTransitionDuration = 0.35f;

	float ArcaneBlendAlpha = 0.f;
	float ArcaneBlendTarget = 0.f;

	float VignetteFlashAlpha = 0.f;
	float VignetteFlashDuration = 0.5f;
};
