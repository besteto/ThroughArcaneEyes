// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeGridCube.generated.h"

class UStaticMeshComponent;
class UTaeStateComponent;
class UMaterialInterface;

UCLASS()
class THROUGHARCANEEYES_API ATaeGridCube : public AActor
{
	GENERATED_BODY()

public:
	ATaeGridCube();

	// Called by ATaeGridManager before BeginPlay to mark this cube as hidden in the default state
	void SetStartHidden(bool bIsShouldBeHidden);

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void OnArcaneStateChanged(bool bArcaneActive);

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	TObjectPtr<UTaeStateComponent> StateComponent;

	// Assigned in BP_GridCube
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Materials")
	TObjectPtr<UMaterialInterface> ForestMaterial;

	// Assigned in BP_GridCube
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Materials")
	TObjectPtr<UMaterialInterface> ArcaneMaterial;

	bool bStartHidden = false;
};
