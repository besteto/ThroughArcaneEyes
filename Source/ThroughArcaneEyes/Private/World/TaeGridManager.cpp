// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeGridManager.h"
#include "World/TaeGridCube.h"
#include "ThroughArcaneEyes.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeGridManager::ATaeGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATaeGridManager::BeginPlay()
{
	Super::BeginPlay();

	if (!GridCubeClass)
	{
		UE_LOG(LogTae, Warning, TEXT("[GridManager] GridCubeClass is NULL — assign BP_GridCube in the level"));
		return;
	}

	const FVector Origin = GetActorLocation();

	for (int32 X = 0; X < GridDimensions.X; ++X)
	{
		for (int32 Y = 0; Y < GridDimensions.Y; ++Y)
		{
			for (int32 Z = 0; Z < GridDimensions.Z; ++Z)
			{
				const FVector SpawnLocation = Origin + FVector(X, Y, Z) * CubeSize;
				GetWorld()->SpawnActor<ATaeGridCube>(GridCubeClass, SpawnLocation, FRotator::ZeroRotator);
			}
		}
	}
}

#if WITH_EDITOR
EDataValidationResult ATaeGridManager::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!GridCubeClass)
	{
		Context.AddError(FText::FromString(TEXT("GridCubeClass is not set — assign BP_GridCube")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
