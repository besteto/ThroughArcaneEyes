// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "TaeGroveComponent.generated.h"

class UCurveFloat;
class UGameplayEffect;
// Actor headers pick this up transitively; a component header may not
class FDataValidationContext;

// A patch of living land. Standing inside it regenerates mana at a rate derived from its own footprint,
// so a larger restoration pays more. Regen is inhibited while Arcane Vision is active.
//
// A component rather than an actor: M2 hosts it on a hand-placed BP_Grove, and M4 attaches the same
// component to whatever ends up owning restoration state.
UCLASS(ClassGroup = (Tae), meta = (BlueprintSpawnableComponent))
class THROUGHARCANEEYES_API UTaeGroveComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UTaeGroveComponent();

	// Horizontal footprint in square metres, from this component's own scaled box extent
	UFUNCTION(BlueprintPure, Category = "Tae|Grove")
	float GetFootprintAreaSqM() const;

	// Pure curve lookup. Static and world-free so it can be tested directly.
	// A null curve yields zero — a grove with no authored curve grants nothing.
	static float RegenRateForArea(float AreaSqM, const UCurveFloat* Curve);

	UFUNCTION(BlueprintPure, Category = "Tae|Grove")
	float GetRegenPerSecond() const;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Mana per second by footprint area in m². Assign Curve_GroveRegen in BP_Grove.
	UPROPERTY(EditAnywhere, Category = "Tae|Grove")
	TObjectPtr<UCurveFloat> RegenCurve;

	// Defaults to UTaeManaRegenEffect; overridable in BP_Grove
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Grove")
	TSubclassOf<UGameplayEffect> RegenEffectClass;

	// One entry per occupant, so overlapping pawns cannot remove each other's regen
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveRegen;
};
