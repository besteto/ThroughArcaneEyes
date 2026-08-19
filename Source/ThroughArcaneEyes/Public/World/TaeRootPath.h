// Copyright © 2026 Helen Allien Poe. See LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/TaeConnectionTypes.h"
#include "TaeRootPath.generated.h"

class USplineComponent;
class USplineMeshComponent;
class UTaeStateComponent;
class UStaticMesh;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConnectionStateChanged, ATaeRootPath*, Path, ETaeConnectionState, NewState);

// Hidden root connection between two islands. Builds one USplineMeshComponent per spline segment in
// OnConstruction (so it updates live as the spline is edited). Growth determines which segments are
// solid: grown segments stay visible and collidable in both modes; ungrown segments appear only as
// ghosts while Arcane.Vision is active, and never collide.
UCLASS()
class THROUGHARCANEEYES_API ATaeRootPath : public AActor
{
	GENERATED_BODY()

public:
	ATaeRootPath();

	virtual void OnConstruction(const FTransform& Transform) override;

	// Advances growth by DeltaAlpha (negative shrinks). Clamped 0..1; broadcasts only on state change.
	void AdvanceGrowth(float DeltaAlpha);

	float GetGrowthAlpha() const { return GrowthAlpha; }
	ETaeConnectionState GetConnectionState() const { return ConnectionState; }

	UPROPERTY(BlueprintAssignable, Category = "RootPath")
	FOnConnectionStateChanged OnConnectionStateChanged;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void OnArcaneStateChanged(bool bInArcaneActive);

	void RebuildSplineMeshes();

	// Applies GrowthAlpha and Arcane state to segment visibility/collision
	void RefreshSegments();

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

	// Persists across Arcane toggles — growth is permanent and partial
	UPROPERTY(VisibleAnywhere, Category = "RootPath")
	float GrowthAlpha = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "RootPath")
	ETaeConnectionState ConnectionState = ETaeConnectionState::Broken;

	bool bArcaneActive = false;
};
