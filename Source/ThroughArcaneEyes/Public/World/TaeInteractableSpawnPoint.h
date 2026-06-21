// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeInteractableSpawnPoint.generated.h"

class UBillboardComponent;

// Empty marker — ATaeInteractableSpawner rolls against a pool of these and decides which ones actually
// spawn something. Place more of these per island than the number of interactables that should appear.
UCLASS()
class THROUGHARCANEEYES_API ATaeInteractableSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ATaeInteractableSpawnPoint();

private:
	UPROPERTY(VisibleAnywhere, Category = "Interactable")
	TObjectPtr<USceneComponent> Root;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Interactable")
	TObjectPtr<UBillboardComponent> Billboard;
#endif
};
