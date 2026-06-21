// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeWorldManager.generated.h"

class ATaeRootPath;

// Per-level registry of hidden root connections. Each ATaeRootPath reveals itself independently via its
// own UTaeStateComponent — this actor exists for designer overview, not as a propagation hub.
UCLASS()
class THROUGHARCANEEYES_API ATaeWorldManager : public AActor
{
	GENERATED_BODY()

public:
	ATaeWorldManager();

private:
	UPROPERTY(EditAnywhere, Category = "World")
	TArray<TObjectPtr<ATaeRootPath>> RootPaths;
};
