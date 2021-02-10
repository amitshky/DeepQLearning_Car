// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkInterface.h"
#include "Engine/Engine.h"

NetworkInterface::NetworkInterface(int32 numStates, int32 numActions, float gamma, float learningRate, torch::Device device)
	:m_PolicyNet(numStates, numActions),
	 m_TargetNet(numStates, numActions),
	 m_Optimizer(m_PolicyNet->parameters(), torch::optim::AdamOptions(learningRate)),
	 m_Gamma(gamma),
	 m_Device(device)
{

}

NetworkInterface::~NetworkInterface()
{
}

// because model could not be initialized in gpu from the global scope or something
void NetworkInterface::Init()
{
	m_PolicyNet->to(m_Device);
	m_TargetNet->to(m_Device);
	m_TargetNet->eval();
	UE_LOG(LogTemp, Warning, TEXT("NetworkInterface init"));
}

void NetworkInterface::Train(const Experience& sample)
{
	// we take the predicted Q-value of the action we performed (hence .gather())
	torch::Tensor qValue = m_PolicyNet->forward(sample.State).gather(1, sample.Action).to(m_Device);	// predicted Q-value

	torch::autograd::GradMode::set_enabled(false); // so that targetQ does not gather gradients (grad is not needed for loss function's target tensor)
	torch::Tensor qmax = m_TargetNet->forward(sample.NextState).max_values(1, true).to(m_Device);
	torch::Tensor targetQ = sample.Done * qmax * m_Gamma + sample.Reward;	// target Q-value 
	// if next state terminates then targetQ = Reward (that is the use of Done)
	torch::autograd::GradMode::set_enabled(true);

	//UE_LOG(LogTemp, Warning, TEXT("qValue: %.4f    targetQ: %.4f"), qValue[0].item<float>(), targetQ[0].item<float>());

	m_Loss = torch::mse_loss(qValue, targetQ);
	m_PolicyNet->zero_grad();
	m_Loss.backward();
	m_Optimizer.step();

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(9, 0.0f, FColor::Black, FString::Printf(TEXT("Loss: %.4f"), m_Loss.item<float>()));
}

void NetworkInterface::CloneModel(const std::string& path)
{
	SavePolicyNet(path);
	LoadTargetNet(path);
}

void NetworkInterface::SavePolicyNet(const std::string& path)
{
	torch::save(m_PolicyNet, path);
}

void NetworkInterface::LoadPolicyNet(const std::string& path)
{
	torch::load(m_PolicyNet, path);
}

void NetworkInterface::SaveTargetNet(const std::string& path)
{
	torch::save(m_TargetNet, path);
}

void NetworkInterface::LoadTargetNet(const std::string& path)
{
	torch::load(m_TargetNet, path);
}

void NetworkInterface::SaveOptimizer(const std::string& path)
{
	torch::save(m_Optimizer, path);
}

void NetworkInterface::LoadOptimizer(const std::string& path)
{
	torch::load(m_Optimizer, path);
}

void NetworkInterface::SaveRewardEval(std::vector<torch::Tensor>& vecRewards, const std::string& path)
{
	torch::save(vecRewards, path);
}

void NetworkInterface::LoadRewardEval(std::vector<torch::Tensor>& vecRewards, const std::string& path)
{
	torch::load(vecRewards, path);
}

// this function doesnt work
//void NetworkInterface::LoadStateDict()
//{
//	torch::autograd::GradMode::set_enabled(false);  // make parameters copying possible
//	auto params = m_PolicyNet->named_parameters();
//	auto newParams = m_TargetNet->named_parameters(true);
//
//	for (auto& val : params)
//	{
//		auto& name = val.key();
//		auto* t = newParams.find(name);
//		if (t != nullptr)
//		{
//			t->copy_(val.value().set_requires_grad(false));
//		}
//	}
//	torch::autograd::GradMode::set_enabled(true);
//}

