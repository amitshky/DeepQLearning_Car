// Fill out your copyright notice in the Description page of Project Settings.


#include "QAgent.h"
#include "Engine/Engine.h"

int64_t QAgent::s_CurrentStep = 0; // static

QAgent::QAgent(int32_t numActions, float start, float end, float decay, torch::Device device)
	:m_Start(start), m_End(end), m_Decay(decay), m_NumActions(numActions), m_Device(device),
	m_Generator(0), m_RandomNum(0, 1)
{
	UE_LOG(LogTemp, Warning, TEXT("QAgent init"));
}

QAgent::~QAgent()
{
}

torch::Tensor QAgent::SelectAction(torch::Tensor& state, Dqnet policyNet)
{
	double rate = GetExplorationRate();
	s_CurrentStep++;
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(12, 0.0, FColor::Black, FString::Printf(TEXT("exploration rate = %.4f"), rate));

	if (rate > m_RandomNum(m_Generator))
	{
		//UE_LOG(LogTemp, Warning, TEXT("random action"));
		return torch::randint(0, m_NumActions, { 1 }, m_Device);
	}
	//UE_LOG(LogTemp, Warning, TEXT("policy action"));
	torch::NoGradGuard noGrad;
	return policyNet->forward(state.reshape({ 1, -1 })).argmax(1).to(m_Device);
}

torch::Tensor QAgent::SelectActionEval(torch::Tensor& state, Dqnet policyNet)
{
	torch::NoGradGuard noGrad;
	return policyNet->forward(state.reshape({ 1, -1 })).argmax(1).to(m_Device);
}
