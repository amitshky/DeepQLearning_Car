// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include "torch/torch.h"
THIRD_PARTY_INCLUDES_END
#include <random>
#include "Dqnet.h"

/**
 *
 */
class DQN_CAR_API QAgent
{
public:
	QAgent(int32_t numActions, float start, float end, float decay, torch::Device device = torch::kCPU);
	~QAgent();

	torch::Tensor SelectAction(torch::Tensor& state, Dqnet policyNet); // select action during training
	torch::Tensor SelectActionEval(torch::Tensor& state, Dqnet policyNet); // select action during evaluation

private:
	FORCEINLINE double GetExplorationRate()
	{
		return m_End + (m_Start - m_End) * std::exp(-1 * s_CurrentStep * m_Decay);
	}

private:
	float m_Start;
	float m_End;
	float m_Decay;
	static int64_t s_CurrentStep;
	int32_t m_NumActions;

	torch::Device m_Device;

	std::mt19937 m_Generator;
	std::uniform_real_distribution<float> m_RandomNum;
};
