// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeRootPath.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TaeStateComponent.h"

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
	for (USplineMeshComponent* Segment : SplineMeshSegments)
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
		Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Segment->SetVisibility(false);

		SplineMeshSegments.Add(Segment);
	}
}

void ATaeRootPath::BeginPlay()
{
	Super::BeginPlay();

	StateComponent->OnArcaneStateChanged.AddDynamic(this, &ATaeRootPath::OnArcaneStateChanged);
}

void ATaeRootPath::OnArcaneStateChanged(bool bArcaneActive)
{
	for (USplineMeshComponent* Segment : SplineMeshSegments)
	{
		if (Segment)
		{
			Segment->SetVisibility(bArcaneActive);
			Segment->SetCollisionEnabled(bArcaneActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		}
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
