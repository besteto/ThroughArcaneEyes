// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "Core/TaeGameInstance.h"
#include "ThroughArcaneEyes.h"
#include "UI/TaeHudViewModel.h"
#include "MVVMGameSubsystem.h"
#include "Types/MVVMViewModelCollection.h"
#include "Types/MVVMViewModelContext.h"

const FName UTaeGameInstance::HudViewModelContextName(TEXT("HudViewModel"));

void UTaeGameInstance::Init()
{
	Super::Init();
	HudViewModel = NewObject<UTaeHudViewModel>(this);

	// Publish the one instance the controller and cue notifies write to. Registered after Super::Init
	// so the subsystem collection exists. Every failure here is logged rather than ignored: the widget
	// reports only "source not found", which cannot distinguish a missing subsystem from a rejected
	// context from this never running at all.
	UMVVMGameSubsystem* Mvvm = GetSubsystem<UMVVMGameSubsystem>();
	if (!Mvvm)
	{
		UE_LOG(LogTae, Warning, TEXT("[GI] UMVVMGameSubsystem not found — WBP_HUD will not resolve '%s'"), *HudViewModelContextName.ToString());
		return;
	}

	UMVVMViewModelCollectionObject* Collection = Mvvm->GetViewModelCollection();
	if (!Collection)
	{
		UE_LOG(LogTae, Warning, TEXT("[GI] MVVM viewmodel collection is NULL — WBP_HUD will not resolve '%s'"), *HudViewModelContextName.ToString());
		return;
	}

	FMVVMViewModelContext Context;
	Context.ContextClass = UTaeHudViewModel::StaticClass();
	Context.ContextName = HudViewModelContextName;

	if (Collection->AddViewModelInstance(Context, HudViewModel))
	{
		UE_LOG(LogTae, Log, TEXT("[GI] Registered HUD viewmodel as '%s'"), *HudViewModelContextName.ToString());
	}
	else
	{
		UE_LOG(LogTae, Warning, TEXT("[GI] AddViewModelInstance rejected '%s' — duplicate identifier or incompatible context"), *HudViewModelContextName.ToString());
	}
}

void UTaeGameInstance::Shutdown()
{
	Super::Shutdown();
}
