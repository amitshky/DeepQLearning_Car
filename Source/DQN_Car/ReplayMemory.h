// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include "torch/torch.h"
THIRD_PARTY_INCLUDES_END
#include <vector>

/**
 * 
 */
class DQN_CAR_API ReplayMemory
{
public:
	ReplayMemory();
	~ReplayMemory();

	void Push(float val);


private:
	 
};
