// Fill out your copyright notice in the Description page of Project Settings.


#include "TrainingGM.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "VehiclePawn.h"
#include "CarGI.h"


ATrainingGM::ATrainingGM()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Car_BP/Sedan/bp_Sedan"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ATrainingGM::BeginPlay()
{
	Super::BeginPlay();
}

void ATrainingGM::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(2, 0.0, FColor::Cyan, FString::Printf(TEXT("Epoch: %d"), UCarGI::EpochCount + 1));

	AVehiclePawn* MyVehicle = Cast<AVehiclePawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyVehicle)
	{
		bool done = MyVehicle->GetDone();
		MyVehicle->TraceDistance(true);
		if (done)
		{
			UCarGI::EpochCount++;
			UGameplayStatics::OpenLevel(MyVehicle, *GetWorld()->GetName());
		}
	}
}
