// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeRootAnchor.generated.h"

class ATaeRootPath;
class USphereComponent;

// The spot Ant stands to grow a root. One at each end of an ATaeRootPath; the player overlaps it and
// channels UGA_GrowRoot. Placed by a designer next to the island edge.
UCLASS()
class THROUGHARCANEEYES_API ATaeRootAnchor : public AActor
{
	GENERATED_BODY()

public:
	ATaeRootAnchor();

	ATaeRootPath* GetPath() const { return Path; }
	float GetGrowthDirection() const { return bGrowsForward ? 1.f : -1.f; }

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UPROPERTY(VisibleAnywhere, Category = "RootAnchor")
	TObjectPtr<USphereComponent> ChannelRange;

	// The connection this anchor grows — assign in the level
	UPROPERTY(EditInstanceOnly, Category = "RootAnchor")
	TObjectPtr<ATaeRootPath> Path;

	// Reserved for choosing which end of the spline materialises first in a later milestone.
	// Not consumed yet — growth currently advances the shared alpha the same way from either end.
	UPROPERTY(EditInstanceOnly, Category = "RootAnchor")
	bool bGrowsForward = true;
};
