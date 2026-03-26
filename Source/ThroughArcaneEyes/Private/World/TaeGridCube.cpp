// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeGridCube.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TaeStateComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeGridCube::ATaeGridCube()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	StateComponent = CreateDefaultSubobject<UTaeStateComponent>(TEXT("StateComponent"));
}

void ATaeGridCube::SetStartHidden(const bool bIsShouldBeHidden)
{
	bStartHidden = bIsShouldBeHidden;
}

void ATaeGridCube::BeginPlay()
{
	Super::BeginPlay();

	StateComponent->OnArcaneStateChanged.AddDynamic(this, &ATaeGridCube::OnArcaneStateChanged);

	if (bStartHidden)
	{
		MeshComponent->SetVisibility(false);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ATaeGridCube::OnArcaneStateChanged(bool bArcaneActive)
{
	if (bStartHidden)
	{
		MeshComponent->SetVisibility(bArcaneActive);
		MeshComponent->SetCollisionEnabled(bArcaneActive
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision);
		// Hidden cubes always use the Arcane material when revealed
		MeshComponent->SetMaterial(0, ArcaneMaterial);
	}
	else
	{
		MeshComponent->SetMaterial(0, bArcaneActive ? ArcaneMaterial : ForestMaterial);
	}
}

#if WITH_EDITOR
EDataValidationResult ATaeGridCube::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!ForestMaterial)
	{
		Context.AddError(FText::FromString(TEXT("ForestMaterial is not set — assign M_GridCube_Forest in BP_GridCube")));
		Result = EDataValidationResult::Invalid;
	}
	if (!ArcaneMaterial)
	{
		Context.AddError(FText::FromString(TEXT("ArcaneMaterial is not set — assign M_GridCube_Arcane in BP_GridCube")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
