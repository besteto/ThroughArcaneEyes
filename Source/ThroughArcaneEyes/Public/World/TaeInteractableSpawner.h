// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeInteractableSpawner.generated.h"

class ATaeInteractableSpawnPoint;
struct FRandomStream;

// One entry in the weighted pool ATaeInteractableSpawner picks from.
USTRUCT(BlueprintType)
struct FTaeInteractableEntry
{
	GENERATED_BODY()

	// Actor to spawn — e.g. BP_Chest, BP_Pickup_Mana
	UPROPERTY(EditAnywhere, Category = "Interactable")
	TSubclassOf<AActor> InteractableClass;

	// Relative likelihood versus other entries in the pool — higher spawns more often
	UPROPERTY(EditAnywhere, Category = "Interactable")
	float Weight = 1.f;
};

// Rolls SpawnChance against every assigned spawn point; points that hit get a random interactable
// from InteractablePool, weighted by FTaeInteractableEntry::Weight.
UCLASS()
class THROUGHARCANEEYES_API ATaeInteractableSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATaeInteractableSpawner();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	TSubclassOf<AActor> PickWeightedClass(FRandomStream& Stream) const;

	// Markers this spawner rolls against — assign all ATaeInteractableSpawnPoint actors for this island
	UPROPERTY(EditAnywhere, Category = "Interactable")
	TArray<TObjectPtr<ATaeInteractableSpawnPoint>> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Interactable")
	TArray<FTaeInteractableEntry> InteractablePool;

	// Chance, per spawn point, that it spawns anything at all
	UPROPERTY(EditAnywhere, Category = "Interactable", meta = (ClampMin = "0", ClampMax = "1"))
	float SpawnChance = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Interactable")
	int32 Seed = 0;
};
