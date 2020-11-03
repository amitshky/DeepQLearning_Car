// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplayMemory.h"


ReplayMemory::ReplayMemory(uint64_t capacity, int32_t numStates, torch::Device device)
	:m_Capacity(capacity), m_NumStates(numStates), m_Device(device), m_Generator(0)
{
	UE_LOG(LogTemp, Warning, TEXT("Replay memory init"));
}

ReplayMemory::~ReplayMemory()
{
}

void ReplayMemory::Push(const Experience& exp)
{
	if (m_Memory.size() < m_Capacity)
	{
		m_Memory.push_back(exp);
		m_RandomNumList.push_back(m_PushCount);
	}
	else
		m_Memory[(m_PushCount % m_Capacity)] = exp;

	m_PushCount++;
	//UE_LOG(LogTemp, Warning, TEXT("Pushed at: %d    push count: %d"), m_PushCount % m_Capacity, m_PushCount);
}

Experience ReplayMemory::Sample(int32_t batchSize)
{
	torch::Tensor states     = torch::zeros({ batchSize, m_NumStates }, m_Device);
	torch::Tensor actions    = torch::zeros({ batchSize, 1 }, torch::kInt64).to(m_Device); // has to be int64 for torch::gather() index
	torch::Tensor nextStates = torch::zeros({ batchSize, m_NumStates }, m_Device);
	torch::Tensor rewards    = torch::zeros({ batchSize, 1 }, m_Device);
	torch::Tensor done       = torch::zeros({ batchSize, 1 }, torch::kInt8).to(m_Device);

	std::shuffle(m_RandomNumList.begin(), m_RandomNumList.end(), m_Generator); // to generate random number without repeat

	//UE_LOG(LogTemp, Warning, TEXT("Sampled"));
	for (int i = 0; i < batchSize; i++)
	{
		int64_t index = m_RandomNumList[i];

		states[i]     = m_Memory[index].State;
		actions[i]    = m_Memory[index].Action;
		nextStates[i] = m_Memory[index].NextState;
		rewards[i]    = m_Memory[index].Reward; // not indexed by 0 because it is set only using torch::tensor without specifying dimension
		done[i]       = m_Memory[index].Done;
	}

	return Experience(states, actions, nextStates, rewards, done);
}

void ReplayMemory::SaveReplayMem(int32_t size, const std::string& path)
{
	if (size >= m_Capacity)
		size = m_Capacity;
	torch::Tensor states     = torch::zeros({ size, m_NumStates }, m_Device);
	torch::Tensor actions    = torch::zeros({ size, 1 }, torch::kInt64).to(m_Device);
	torch::Tensor nextStates = torch::zeros({ size, m_NumStates }, m_Device);
	torch::Tensor rewards    = torch::zeros({ size, 1 }, m_Device);
	torch::Tensor done       = torch::zeros({ size, 1 }, torch::kInt8).to(m_Device);

	for (int i = 0; i < size; i++)
	{
		states[i]     = m_Memory[i].State;
		actions[i]    = m_Memory[i].Action;
		nextStates[i] = m_Memory[i].NextState;
		rewards[i]    = m_Memory[i].Reward;
		done[i]       = m_Memory[i].Done;
	}

	torch::save(states,     path + "State.pt");
	torch::save(actions,    path + "Action.pt");
	torch::save(nextStates, path + "NextState.pt");
	torch::save(rewards,    path + "Reward.pt");
	torch::save(done,       path + "Done.pt");
}

void ReplayMemory::LoadReplayMem(const std::string& path)
{
	torch::Tensor states, actions, nextStates, rewards, done;

	torch::load(states,     path + "State.pt");
	torch::load(actions,    path + "Action.pt");
	torch::load(nextStates, path + "NextState.pt");
	torch::load(rewards,    path + "Reward.pt");
	torch::load(done,       path + "Done.pt");

	for (int64_t i = 0; i < states.size(0); i++)
	{
		Push(Experience(states[i], actions[i], nextStates[i], rewards[i], done[i]));
	}
}
