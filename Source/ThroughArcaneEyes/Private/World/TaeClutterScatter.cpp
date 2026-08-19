// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "World/TaeClutterScatter.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeClutterScatter::ATaeClutterScatter()
{
	PrimaryActorTick.bCanEverTick = false;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	SetRootComponent(Bounds);
	Bounds->SetBoxExtent(FVector(500.f, 500.f, 100.f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATaeClutterScatter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ScatterInstances();
}

void ATaeClutterScatter::ScatterInstances()
{
	for (UInstancedStaticMeshComponent* Comp : ScatterComponents)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	ScatterComponents.Reset();

	if (MeshPool.IsEmpty())
	{
		return;
	}

	ScatterComponents.Reserve(MeshPool.Num());
	for (const TObjectPtr<UStaticMesh>& Mesh : MeshPool)
	{
		UInstancedStaticMeshComponent* Comp = NewObject<UInstancedStaticMeshComponent>(this);
		Comp->SetStaticMesh(Mesh);
		Comp->SetupAttachment(Bounds);
		Comp->RegisterComponent();
		AddInstanceComponent(Comp);
		ScatterComponents.Add(Comp);
	}

	FRandomStream Stream(Seed);
	const FVector Extent = Bounds->GetUnscaledBoxExtent();

	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		const int32 MeshIndex = Stream.RandRange(0, MeshPool.Num() - 1);
		UInstancedStaticMeshComponent* Comp = ScatterComponents[MeshIndex];
		if (!Comp || !Comp->GetStaticMesh())
		{
			continue;
		}

		FVector LocalLocation(
			Stream.FRandRange(-Extent.X, Extent.X),
			Stream.FRandRange(-Extent.Y, Extent.Y),
			Extent.Z);

		if (bSnapToGround)
		{
			const FVector WorldStart = GetActorTransform().TransformPosition(LocalLocation);
			const FVector WorldEnd = WorldStart - FVector(0.f, 0.f, GroundTraceDistance);
			FHitResult Hit;
			// Ignore self — the ISM components already hold every instance placed so far this pass, and
			// their collision is on by default, so without this each prop can land on top of an earlier one.
			FCollisionQueryParams TraceParams;
			TraceParams.AddIgnoredActor(this);
			if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Hit, WorldStart, WorldEnd, ECC_Visibility, TraceParams))
			{
				LocalLocation = GetActorTransform().InverseTransformPosition(Hit.Location);
			}
		}

		const float Yaw = Stream.FRandRange(0.f, 360.f);
		const float Scale = Stream.FRandRange(UniformScaleRange.X, UniformScaleRange.Y);
		const FTransform InstanceTransform(FRotator(0.f, Yaw, 0.f), LocalLocation, FVector(Scale));

		Comp->AddInstance(InstanceTransform);
	}
}

#if WITH_EDITOR
EDataValidationResult ATaeClutterScatter::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (MeshPool.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("MeshPool is empty — assign at least one small prop mesh")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
