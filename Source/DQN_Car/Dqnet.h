// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include "torch/torch.h"
THIRD_PARTY_INCLUDES_END

/**
 *
 */
class DQN_CAR_API DqnetImpl : public torch::nn::Module
{
public:
	DqnetImpl(int32_t numStates, int32_t numActions);
	~DqnetImpl();

	torch::Tensor forward(torch::Tensor x);

private:
	torch::nn::Linear m_Linear1;
	torch::nn::Linear m_Linear2;
	torch::nn::Linear m_Out;
};
TORCH_MODULE(Dqnet);
