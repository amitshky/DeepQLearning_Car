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
		torch::Tensor state = MyVehicle->GetState();
		torch::Tensor action = UCarGI::Agent->SelectAction(state, UCarGI::Net->GetPolicyNet());

		MyVehicle->TakeAction(action);
		torch::Tensor nextState = MyVehicle->GetState();
		torch::Tensor reward = MyVehicle->GetReward();

		bool done = MyVehicle->GetDone();
		torch::Tensor doneTensor = torch::tensor(done ? 0 : 1, UCarGI::Device); // we need 0 when true so this is correct

		UCarGI::EpRewards += reward;

		Experience exp(state, action, nextState, reward, doneTensor);
		UCarGI::Mem->Push(exp);
		if (UCarGI::Mem->CanProvideSample(UCarGI::BatchSize))
		{
			exp = UCarGI::Mem->Sample(UCarGI::BatchSize);
			UCarGI::Net->Train(exp);
		}

		if (UCarGI::StepCount++ == UCarGI::UpdateStep)
		{
			// update targetNet
			UCarGI::StepCount = 0;
			UCarGI::Net->CloneModel(UCarGI::FilePath);
			UE_LOG(LogTemp, Warning, TEXT("Model Updated    epoch = %d"), UCarGI::EpochCount + 1);
		}

		if (done)	// end of episode
		{
			constexpr int32_t updateFreq = 20;
			if (UCarGI::EpochCount % updateFreq == 0.0f)
			{
				UCarGI::VecRewards.push_back(UCarGI::EpRewards / updateFreq);
				UCarGI::EpRewards = torch::zeros({ 1 });
			}

			UCarGI::EpochCount++;
			UGameplayStatics::OpenLevel(MyVehicle, *GetWorld()->GetName());
		}
	}
}
