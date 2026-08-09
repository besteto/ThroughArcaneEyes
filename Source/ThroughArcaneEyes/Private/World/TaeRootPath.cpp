// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeRootPath.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TaeStateComponent.h"
#include "World/TaeConnectionTypes.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeRootPath::ATaeRootPath()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(Spline);

	StateComponent = CreateDefaultSubobject<UTaeStateComponent>(TEXT("StateComponent"));
}

void ATaeRootPath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildSplineMeshes();
}

void ATaeRootPath::RebuildSplineMeshes()
{
	// Destroy EVERY spline mesh this actor owns, not just the ones the tracking array knows about.
	// SplineMeshSegments is Transient, so it is empty after PIE duplication and after a construction
	// rerun — while the duplicated components themselves survive. Iterating the array here therefore
	// destroyed nothing and leaked a full orphaned set on every rebuild.
	TArray<USplineMeshComponent*> ExistingSegments;
	GetComponents(ExistingSegments);
	for (USplineMeshComponent* Segment : ExistingSegments)
	{
		if (Segment)
		{
			Segment->DestroyComponent();
		}
	}
	SplineMeshSegments.Reset();

	if (!PathMesh)
	{
		return;
	}

	const int32 NumSegments = Spline->GetNumberOfSplinePoints() - 1;
	SplineMeshSegments.Reserve(FMath::Max(NumSegments, 0));

	for (int32 Index = 0; Index < NumSegments; ++Index)
	{
		USplineMeshComponent* Segment = NewObject<USplineMeshComponent>(this);
		Segment->SetMobility(EComponentMobility::Movable);
		Segment->SetStaticMesh(PathMesh);
		if (PathMaterial)
		{
			Segment->SetMaterial(0, PathMaterial);
		}
		Segment->SetupAttachment(Spline);
		Segment->RegisterComponent();
		AddInstanceComponent(Segment);

		const FVector StartPos = Spline->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::Local);
		const FVector StartTangent = Spline->GetTangentAtSplinePoint(Index, ESplineCoordinateSpace::Local);
		const FVector EndPos = Spline->GetLocationAtSplinePoint(Index + 1, ESplineCoordinateSpace::Local);
		const FVector EndTangent = Spline->GetTangentAtSplinePoint(Index + 1, ESplineCoordinateSpace::Local);

		Segment->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

		SplineMeshSegments.Add(Segment);
	}
}

void ATaeRootPath::BeginPlay()
{
	Super::BeginPlay();

	StateComponent->OnArcaneStateChanged.AddDynamic(this, &ATaeRootPath::OnArcaneStateChanged);

	// SplineMeshSegments is Transient and does not survive PIE duplication or a construction rerun, so
	// by the time play starts the array is empty even though the components exist. Rebuild so the array
	// again matches reality — without this, RefreshSegments early-returns and the root never reveals.
	if (SplineMeshSegments.Num() == 0)
	{
		RebuildSplineMeshes();
	}

	// Segments stay visible in the editor so the spline can be authored; they only hide once play starts,
	// same as ATaeGridCube's bStartHidden handling.
	RefreshSegments();
}

void ATaeRootPath::AdvanceGrowth(const float DeltaAlpha)
{
	const float NewAlpha = FTaeGrowthStep::Advance(GrowthAlpha, DeltaAlpha);
	if (NewAlpha == GrowthAlpha)
	{
		return;
	}

	GrowthAlpha = NewAlpha;

	const ETaeConnectionState NewState = FTaeGrowthStep::StateFor(GrowthAlpha);
	const bool bStateChanged = NewState != ConnectionState;
	ConnectionState = NewState;

	RefreshSegments();

	if (bStateChanged)
	{
		OnConnectionStateChanged.Broadcast(this, ConnectionState);
	}
}

void ATaeRootPath::OnArcaneStateChanged(const bool bInArcaneActive)
{
	bArcaneActive = bInArcaneActive;
	RefreshSegments();
}

void ATaeRootPath::RefreshSegments()
{
	const int32 NumSegments = SplineMeshSegments.Num();
	if (NumSegments == 0)
	{
		return;
	}

	// Segments materialise in order as the root grows
	const float GrownSegments = GrowthAlpha * static_cast<float>(NumSegments);

	for (int32 Index = 0; Index < NumSegments; ++Index)
	{
		USplineMeshComponent* Segment = SplineMeshSegments[Index];
		if (!Segment)
		{
			continue;
		}

		const bool bGrown = static_cast<float>(Index) < GrownSegments;

		// Grown segments are solid in both modes; ungrown ones are ghosts only Arcane can see
		const bool bVisible = bGrown || bArcaneActive;
		const bool bCollides = bGrown;

		Segment->SetVisibility(bVisible);
		Segment->SetCollisionEnabled(bCollides
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision);
	}
}

#if WITH_EDITOR
EDataValidationResult ATaeRootPath::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!PathMesh)
	{
		Context.AddError(FText::FromString(TEXT("PathMesh is not set — assign the root/vine mesh in BP_RootPath")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
