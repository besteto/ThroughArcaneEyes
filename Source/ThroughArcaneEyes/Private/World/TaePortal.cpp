// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaePortal.h"
#include "Components/SphereComponent.h"
#include "Core/TaeHud.h"
#include "Character/TaeCharacter.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#endif

ATaePortal::ATaePortal()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetSphereRadius(120.f);
	TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(TriggerSphere);

#if WITH_EDITORONLY_DATA
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(TriggerSphere);
#endif
}

void ATaePortal::BeginPlay()
{
	Super::BeginPlay();
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ATaePortal::OnOverlapBegin);
}

void ATaePortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<ATaeCharacter>(OtherActor)) return;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ATaeHud* Hud = PC->GetHUD<ATaeHud>())
		{
			Hud->ShowVictoryScreen();
		}
	}
}
