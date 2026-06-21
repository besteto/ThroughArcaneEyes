// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeRootPath.generated.h"

class USplineComponent;
class USplineMeshComponent;
class UTaeStateComponent;
class UStaticMesh;
class UMaterialInterface;

// Hidden root connection between two islands. Builds one USplineMeshComponent per spline segment in
// OnConstruction (so it updates live as the spline is edited); reveal follows the same pattern as
// ATaeGridCube — a UTaeStateComponent listens for Arcane.Vision and toggles visibility/collision.
UCLASS()
class THROUGHARCANEEYES_API ATaeRootPath : public AActor
{
	GENERATED_BODY()

public:
	ATaeRootPath();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void OnArcaneStateChanged(bool bArcaneActive);

	void RebuildSplineMeshes();

	UPROPERTY(VisibleAnywhere, Category = "RootPath")
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(VisibleAnywhere, Category = "RootPath")
	TObjectPtr<UTaeStateComponent> StateComponent;

	// Root/vine mesh extruded along the spline — assign in BP_RootPath
	UPROPERTY(EditAnywhere, Category = "RootPath")
	TObjectPtr<UStaticMesh> PathMesh;

	UPROPERTY(EditAnywhere, Category = "RootPath")
	TObjectPtr<UMaterialInterface> PathMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshSegments;
};
