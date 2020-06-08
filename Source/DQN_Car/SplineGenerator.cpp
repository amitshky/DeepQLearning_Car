// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineGenerator.h"

// Sets default values
ASplineGenerator::ASplineGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(FName(TEXT("Spline")));
	SplineComponent->SetRelativeLocation(FVector::ZeroVector);
	SplineComponent->SetRelativeScale3D(FVector(1));
	SplineComponent->bShouldVisualizeScale = true;
	this->SetRootComponent(SplineComponent);
}

// Called when the game starts or when spawned
void ASplineGenerator::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASplineGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASplineGenerator::OnConstruction(const FTransform& Transform)
{
	// no spawn for preview actors
	if (!HasAnyFlags(EObjectFlags::RF_Transient))
	{
		GetWorldTimerManager().SetTimer(hSpawnTimer, this, &ASplineGenerator::OnSpawn, 0.2f, false);
	}
}

void ASplineGenerator::OnSpawn()
{
	GenerateComponents(SplineComponent);
}

void ASplineGenerator::GenerateComponents(USplineComponent* spline)
{
	if (!spline)
		return;

	FVector locStart, tanStart, locEnd, tanEnd, ss, es;
	FVector2D sscale;
	FVector2D escale;
	sscale = escale = scale;

	// clean up stale mesh components
	if (Components.Num() > 0)
	{
		for (int32 i = 0; i < Components.Num(); i++)
		{
			if (Components[i])
			{
				//Components[i]->DetachFromParent(); // depricated function
				Components[i]->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				Components[i]->DestroyComponent();
			}
		}
		Components.Empty();
	}

	if (ShowMesh && SegmentLength > 1)
	{
		float total = spline->GetSplineLength();
		TArray<FVector2D> segs;
		float current = 0;
		while (current < total)
		{
			segs.Add(FVector2D(current, UKismetMathLibrary::FMin(current + SegmentLength, total)));
			current += SegmentLength;
		}
		for (int32 i = 0; i < segs.Num(); i++)
		{
			USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this);
			splineMesh->RegisterComponent();
			//UE_LOG(LogTemp, Display, TEXT("Added new USplineMeshComponent: %s"), *splineMesh->GetName());
			splineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			GetLTSAtDistance(segs[i].X, locStart, tanStart, ss);
			GetLTSAtDistance(segs[i].Y, locEnd, tanEnd, es);
			tanStart = tanStart.GetClampedToMaxSize(SegmentLength);
			tanEnd = tanEnd.GetClampedToMaxSize(SegmentLength);
			//UE_LOG(LogTemp, Display, TEXT("Start/End location and tangent: %d %s / %s"), i, *locStart.ToString(), *locEnd.ToString());

			splineMesh->SetMobility(EComponentMobility::Type::Movable);
			splineMesh->SetForwardAxis(ForwardAxis);
			splineMesh->SetStaticMesh(Mesh);
			splineMesh->SetMaterial(0, Material);
			splineMesh->SetStartAndEnd(locStart, tanStart, locEnd, tanEnd);
			splineMesh->SetStartScale(FVector2D(ss.Y, ss.Z));
			splineMesh->SetEndScale(FVector2D(es.Y, es.Z));
			splineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			splineMesh->AttachToComponent(spline, FAttachmentTransformRules::FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
			Components.Add(splineMesh);
		}
	}
}

void ASplineGenerator::GetLTSAtDistance(float distance, FVector& loc, FVector& tan, FVector& scaleVal)
{
	loc = SplineComponent->GetLocationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);
	tan = SplineComponent->GetTangentAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);
	scaleVal = SplineComponent->GetScaleAtDistanceAlongSpline(distance);
}

