// Fill out your copyright notice in the Description page of Project Settings.


#include "DQN_Car_GI.h"


// private
int32 UDQN_Car_GI::capacity = 128 * 1024;
std::string UDQN_Car_GI::rootPath = "C:/_My Files/Unreal Engine/Vehicle/SavedNets/"; // change this to your local directory

// public
int32 UDQN_Car_GI::batchSize = 128;
int32 UDQN_Car_GI::numStates = 5;
int32 UDQN_Car_GI::numActions = 3;
int32 UDQN_Car_GI::epochCount = 0;
int64 UDQN_Car_GI::stepCount = 0;

const std::string UDQN_Car_GI::filePath = rootPath + "policy.pt";


UDQN_Car_GI::UDQN_Car_GI()
{
	float start = 0.9f;
	float end = 0.1f;
	float decay = 1e-4f;
	float gamma = 0.95f;
	float learningRate = 1e-3f;
	bool resume = true; // to resume training // also make sure the previous model has the same architecture before setting it to "true"

	// initialize mem, agent and net

}

UDQN_Car_GI::~UDQN_Car_GI()
{
	// save model and experience
}
