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

void UTaeArcaneSubsystem::SetArcaneActive(bool bActive)
{
	bVolumeActive = bActive;

	if (SpectralVolume)
	{
		SpectralVolume->bEnabled = bActive;
	}

	CrossfadeMusic(bActive);
	FlashVignette();
}

void UTaeArcaneSubsystem::FlashVignette(float Duration)
{
	if (!SpectralVolume) return;

	// Spike vignette to max then fade out over Duration
	FPostProcessSettings& Settings = SpectralVolume->Settings;
	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = 1.f;

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(VignetteTimerHandle);
	World->GetTimerManager().SetTimer(
		VignetteTimerHandle,
		this,
		&UTaeArcaneSubsystem::ClearVignetteFlash,
		Duration,
		false
	);
}

void UTaeArcaneSubsystem::ClearVignetteFlash()
{
	if (!SpectralVolume) return;
	SpectralVolume->Settings.VignetteIntensity = 0.f;
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
