// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeClutterScatter.generated.h"

class UBoxComponent;
class UStaticMesh;
class UInstancedStaticMeshComponent;

// Scatters small static meshes (rocks, wire piles, scrap) inside Bounds, one ISM component per mesh.
// Re-runs on every OnConstruction so edits preview live in the editor; Seed keeps the layout stable
// until changed.
UCLASS()
class THROUGHARCANEEYES_API ATaeClutterScatter : public AActor
{
	GENERATED_BODY()

public:
	ATaeClutterScatter();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	void ScatterInstances();

	UPROPERTY(VisibleAnywhere, Category = "Clutter")
	TObjectPtr<UBoxComponent> Bounds;

	// Small props to scatter — assign in BP_ClutterScatter_Rocks / BP_ClutterScatter_Wires
	UPROPERTY(EditAnywhere, Category = "Clutter")
	TArray<TObjectPtr<UStaticMesh>> MeshPool;

	UPROPERTY(EditAnywhere, Category = "Clutter")
	int32 InstanceCount = 20;

	// Same seed always produces the same layout — change it to reroll
	UPROPERTY(EditAnywhere, Category = "Clutter")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, Category = "Clutter")
	FVector2D UniformScaleRange = FVector2D(0.8f, 1.2f);

	// Drops each instance onto the first hit beneath it — needs the island mesh to have collision
	UPROPERTY(EditAnywhere, Category = "Clutter")
	bool bSnapToGround = false;

	UPROPERTY(EditAnywhere, Category = "Clutter", meta = (EditCondition = "bSnapToGround"))
	float GroundTraceDistance = 1000.f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> ScatterComponents;
};
