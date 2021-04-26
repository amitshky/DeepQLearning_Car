// Fill out your copyright notice in the Description page of Project Settings.


#include "EvaluationGM.h"

// Fill out your copyright notice in the Description page of Project Settings.


#include "EvaluationGM.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "CarGI.h"
#include "VehiclePawn.h"
#include "Engine/Engine.h"


AEvaluationGM::AEvaluationGM()
{
	PrimaryActorTick.bCanEverTick = true;

	// set default pawn class to the player
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Car_BP/Sedan/bp_Sedan"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AEvaluationGM::BeginPlay()
{
	Super::BeginPlay();

}

void AEvaluationGM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(10, 0.0, FColor::Black, FString::Printf(TEXT("Epoch: %d"), UCarGI::EpochCountEval + 1));

	AVehiclePawn* MyVehicle = Cast<AVehiclePawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyVehicle)
	{
		torch::Tensor state = MyVehicle->GetState();
		torch::NoGradGuard noGrad;
		torch::Tensor action = UCarGI::Agent->SelectActionEval(state, UCarGI::Net->GetPolicyNet());

		MyVehicle->TakeAction(action);

		bool done = MyVehicle->GetDone();

		if (done)	// end of episode
		{
			UCarGI::EpochCountEval++;
			UGameplayStatics::OpenLevel(MyVehicle, *GetWorld()->GetName());
		}
	}
}
