// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeGridManager.generated.h"

class ATaeGridCube;

UCLASS()
class THROUGHARCANEEYES_API ATaeGridManager : public AActor
{
	GENERATED_BODY()

public:
	ATaeGridManager();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	// Number of cubes along X, Y, Z axes
	UPROPERTY(EditAnywhere, Category = "Grid")
	FIntVector GridDimensions = FIntVector(5, 5, 3);

	// World-space size of each cube — must match the static mesh bounds in BP_GridCube
	UPROPERTY(EditAnywhere, Category = "Grid")
	float CubeSize = 100.f;

	// Assign BP_GridCube
	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<ATaeGridCube> GridCubeClass;
};
