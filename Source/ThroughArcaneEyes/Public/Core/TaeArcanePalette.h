// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TaeArcanePalette.generated.h"

// Names of the vector parameters in MPC_Arcane. Setting a parameter the collection does not declare
// fails silently, so the collection asset must declare exactly these four.
namespace TaeArcaneParams
{
	inline const FName SpectralEdge(TEXT("SpectralEdge"));
	inline const FName CubeTint(TEXT("CubeTint"));
	inline const FName GroveBloom(TEXT("GroveBloom"));
	inline const FName GrowthFront(TEXT("GrowthFront"));
}

// The arcane look in one place. Materials read these through MPC_Arcane, the Niagara systems sample
// the same collection, and M3's Slate overlay will read this asset directly — Slate cannot read a
// Material Parameter Collection, which is why the data asset is the source of truth and the
// collection is a derived copy.
//
// Assign DA_ArcanePalette in BP_TaeGameInstance.
UCLASS(BlueprintType)
class THROUGHARCANEEYES_API UTaeArcanePalette : public UDataAsset
{
	GENERATED_BODY()

public:
	// Edge glow on M_SpectralEdge, and the line colour of the M3 network overlay
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor SpectralEdge = FLinearColor(0.2f, 0.8f, 1.f, 1.f);

	// Arcane-mode tint on M_GridCube_Arcane
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor CubeTint = FLinearColor(0.1f, 0.4f, 0.6f, 1.f);

	// NS_GroveBloom — living land, so this one is deliberately warm against the cold arcane palette
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor GroveBloom = FLinearColor(0.4f, 0.9f, 0.35f, 1.f);

	// NS_GrowthFront — the growing tip
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor GrowthFront = FLinearColor(0.6f, 1.f, 0.5f, 1.f);
};
