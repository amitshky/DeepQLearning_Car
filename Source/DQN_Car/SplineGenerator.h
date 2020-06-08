// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SplineGenerator.generated.h"

UCLASS()
class DQN_CAR_API ASplineGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASplineGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
		void OnSpawn();

	UFUNCTION()
		void GenerateComponents(USplineComponent* spline);

	UFUNCTION()
		void GetLTSAtDistance(float distance, FVector& loc, FVector& tan, FVector& scaleVal);

	UPROPERTY()
		TArray<USplineMeshComponent*> Components;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		USplineComponent* SplineComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		bool ShowMesh = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		float SegmentLength = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		FVector2D scale = FVector2D(1, 1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		UStaticMesh* Mesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		UMaterial* Material = nullptr;

private:
	FRandomStream rng;
	FTimerHandle hSpawnTimer;

};
