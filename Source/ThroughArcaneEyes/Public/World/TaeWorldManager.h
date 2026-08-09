// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/TaeConnectionTypes.h"
#include "TaeWorldManager.generated.h"

class ATaeRootPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkChanged, int32, RestoredCount, int32, RequiredCount);

// Per-level registry of root connections. Each ATaeRootPath still owns its own state; this actor
// subscribes to them, keeps the counts, and is the single thing the HUD and win condition observe.
UCLASS()
class THROUGHARCANEEYES_API ATaeWorldManager : public AActor
{
	GENERATED_BODY()

public:
	ATaeWorldManager();

	int32 GetRestoredCount() const { return RestoredCount; }
	int32 GetRequiredCount() const { return RootPaths.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnNetworkChanged OnNetworkChanged;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void HandleConnectionStateChanged(ATaeRootPath* Path, ETaeConnectionState NewState);

	void RecountRestored();

	UPROPERTY(EditInstanceOnly, Category = "World")
	TArray<TObjectPtr<ATaeRootPath>> RootPaths;

	UPROPERTY(VisibleAnywhere, Category = "World")
	int32 RestoredCount = 0;
};
