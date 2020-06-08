// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <string>
#include "DQN_Car_GI.generated.h"

/**
 * 
 */
UCLASS()
class DQN_CAR_API UDQN_Car_GI : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UDQN_Car_GI();
	~UDQN_Car_GI();

	// Hyperparameters
	static int32 batchSize;
	static int32 numStates;
	static int32 numActions;
	static int32 epochCount;
	static int64 stepCount;

	const static std::string filePath;

	// declare members mem, agent and net

private:
	static int32 capacity;
	static std::string rootPath;
};
