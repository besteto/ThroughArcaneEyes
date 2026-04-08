// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaePortal.generated.h"

class USphereComponent;
class UBillboardComponent;

UCLASS()
class THROUGHARCANEEYES_API ATaePortal : public AActor
{
	GENERATED_BODY()

public:
	ATaePortal();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// Trigger radius — adjust in BP_Portal to fit the island landing spot
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	TObjectPtr<USphereComponent> TriggerSphere;

#if WITH_EDITORONLY_DATA
	// Visible marker in editor so you can see where the portal is placed
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	TObjectPtr<UBillboardComponent> Billboard;
#endif
};
