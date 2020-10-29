// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
//#include "ReplayMemory.h"
//#include "QAgent.h"
#include "NetworkInterface.h"
#include <string>
#include <memory>
#include "CarGI.generated.h"

/**
 * 
 */
UCLASS()
class DQN_CAR_API UCarGI : public UGameInstance
{
	GENERATED_BODY()
public:
	UCarGI();
	~UCarGI();

	static const uint64 Capacity;  // capacity of Replay memory
	static const int32 BatchSize;  // batch size for training
	static const int32 NumStates;  // number of states
	static const int32 NumActions; // number of actions

	static int32 EpochCount; // epoch/episode count
	static int64 StepCount;  // timestep count

	static const std::string RootPath; // path to SavedNets folder
	static const std::string FilePath; // path to Policy.pt saved inside SavedNets folder

	static torch::Device Device; // device // cuda or cpu

	//static std::unique_ptr<ReplayMemory> Mem;
	//static std::unique_ptr<QAgent> Agent;
	static std::unique_ptr<NetworkInterface> Net;
};
