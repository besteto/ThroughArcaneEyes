// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeInteractableSpawnPoint.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#endif

ATaeInteractableSpawnPoint::ATaeInteractableSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

#if WITH_EDITORONLY_DATA
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(Root);
#endif
}
