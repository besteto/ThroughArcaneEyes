// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeInteractableSpawner.h"
#include "World/TaeInteractableSpawnPoint.h"
#include "Math/RandomStream.h"
#include "ThroughArcaneEyes.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeInteractableSpawner::ATaeInteractableSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATaeInteractableSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (InteractablePool.IsEmpty())
	{
		UE_LOG(LogTae, Warning, TEXT("[InteractableSpawner] InteractablePool is empty — nothing will spawn"));
		return;
	}

	FRandomStream Stream(Seed);

	for (const TObjectPtr<ATaeInteractableSpawnPoint>& Point : SpawnPoints)
	{
		if (!Point || Stream.FRand() > SpawnChance)
		{
			continue;
		}

		if (const TSubclassOf<AActor> Class = PickWeightedClass(Stream))
		{
			GetWorld()->SpawnActor<AActor>(Class, Point->GetActorTransform());
		}
	}
}

TSubclassOf<AActor> ATaeInteractableSpawner::PickWeightedClass(FRandomStream& Stream) const
{
	float TotalWeight = 0.f;
	for (const FTaeInteractableEntry& Entry : InteractablePool)
	{
		TotalWeight += FMath::Max(Entry.Weight, 0.f);
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Roll = Stream.FRandRange(0.f, TotalWeight);
	for (const FTaeInteractableEntry& Entry : InteractablePool)
	{
		Roll -= FMath::Max(Entry.Weight, 0.f);
		if (Roll <= 0.f)
		{
			return Entry.InteractableClass;
		}
	}

	return InteractablePool.Last().InteractableClass;
}

#if WITH_EDITOR
EDataValidationResult ATaeInteractableSpawner::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (SpawnPoints.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("SpawnPoints is empty — assign ATaeInteractableSpawnPoint actors")));
		Result = EDataValidationResult::Invalid;
	}
	if (InteractablePool.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("InteractablePool is empty — assign at least one interactable class")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
