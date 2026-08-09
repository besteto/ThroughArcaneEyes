// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Core/TaeArcaneSubsystem.h"
#include "Core/TaeGameInstance.h"
#include "Engine/PostProcessVolume.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UTaeArcaneSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Auto-find the PostProcessVolume placed in the level
	SpectralVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(&InWorld, APostProcessVolume::StaticClass()));
	if (SpectralVolume)
	{
		SpectralVolume->bEnabled = false;
		SpectralVolume->BlendWeight = 0.f;
	}

	UTaeGameInstance* GI = InWorld.GetGameInstance<UTaeGameInstance>();
	if (!GI) return;

	if (USoundBase* ForestMusic = GI->GetForestMusic())
	{
		ForestMusicComp = UGameplayStatics::SpawnSound2D(&InWorld, ForestMusic, 1.f, 1.f, 0.f, nullptr, true, false);
	}

	if (USoundBase* ArcaneMusic = GI->GetArcaneMusic())
	{
		ArcaneMusicComp = UGameplayStatics::SpawnSound2D(&InWorld, ArcaneMusic, 0.f, 1.f, 0.f, nullptr, true, false);
		if (ArcaneMusicComp)
		{
			ArcaneMusicComp->SetVolumeMultiplier(0.f);
		}
	}
}

float UTaeArcaneSubsystem::StepBlendAlpha(const float Current, const float Target, const float DeltaTime, const float Duration)
{
	if (Duration <= 0.f)
	{
		return Target;
	}

	// A speed of 1/Duration traverses the full 0..1 range in exactly Duration seconds.
	return FMath::FInterpConstantTo(Current, Target, DeltaTime, 1.f / Duration);
}

void UTaeArcaneSubsystem::SetArcaneActive(const bool bActive)
{
	ArcaneBlendTarget = bActive ? 1.f : 0.f;

	CrossfadeMusic(bActive);
	FlashVignette();
}

void UTaeArcaneSubsystem::FlashVignette(const float Duration)
{
	VignetteFlashDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	VignetteFlashAlpha = 1.f;
}

void UTaeArcaneSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float NewAlpha = StepBlendAlpha(ArcaneBlendAlpha, ArcaneBlendTarget, DeltaTime, ArcaneTransitionDuration);
	const bool bAlphaChanged = !FMath::IsNearlyEqual(NewAlpha, ArcaneBlendAlpha);
	ArcaneBlendAlpha = NewAlpha;

	// Vignette now actually interpolates — it used to snap to zero on a timer
	const bool bFlashing = VignetteFlashAlpha > 0.f;
	if (bFlashing)
	{
		VignetteFlashAlpha = FMath::Max(VignetteFlashAlpha - (DeltaTime / VignetteFlashDuration), 0.f);
	}

	if (bAlphaChanged || bFlashing)
	{
		ApplyBlendAlpha();
	}
}

TStatId UTaeArcaneSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTaeArcaneSubsystem, STATGROUP_Tickables);
}

void UTaeArcaneSubsystem::ApplyBlendAlpha()
{
	if (!SpectralVolume)
	{
		return;
	}

	// BlendWeight replaces the old binary bEnabled so the volume fades with the camera
	SpectralVolume->bEnabled = ArcaneBlendAlpha > 0.f;
	SpectralVolume->BlendWeight = ArcaneBlendAlpha;

	FPostProcessSettings& Settings = SpectralVolume->Settings;
	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = VignetteFlashAlpha;
}

void UTaeArcaneSubsystem::CrossfadeMusic(bool bToArcane)
{
	UTaeGameInstance* GI = GetWorld()->GetGameInstance<UTaeGameInstance>();
	const float FadeTime = GI ? GI->GetMusicCrossfadeDuration() : 1.f;

	if (ForestMusicComp)
	{
		if (bToArcane)
			ForestMusicComp->FadeOut(FadeTime, 0.f);
		else
			ForestMusicComp->FadeIn(FadeTime, 1.f);
	}

	if (ArcaneMusicComp)
	{
		if (bToArcane)
			ArcaneMusicComp->FadeIn(FadeTime, 1.f);
		else
			ArcaneMusicComp->FadeOut(FadeTime, 0.f);
	}
}
