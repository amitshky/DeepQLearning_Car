#pragma once

#include "CoreMinimal.h"
THIRD_PARTY_INCLUDES_START
#include "torch/torch.h"
THIRD_PARTY_INCLUDES_END
#include <vector>
#include <random>

struct Experience
{
	Experience() {}
	Experience(const torch::Tensor& state, const torch::Tensor& action, const torch::Tensor& nextState, 
		const torch::Tensor& reward, const torch::Tensor& done)
	{
		State     = state;
		Action    = action;
		NextState = nextState;
		Reward    = reward;
		Done      = done;
	}

	torch::Tensor State;
	torch::Tensor Action;
	torch::Tensor NextState;
	torch::Tensor Reward;
	torch::Tensor Done;
};

// this class is a bitch
// refactor it soon
class DQN_CAR_API ReplayMemory
{
public:
	ReplayMemory(uint64_t capacity, int32_t numStates, torch::Device device = torch::kCPU);
	~ReplayMemory();

	void Push(const Experience& exp);
	Experience Sample(int32_t batchSize);

	void SaveReplayMem(int32_t size, const std::string& path); // save `size` of current replay memory
	void LoadReplayMem(const std::string& path); // Load previously saved states, actions, ...

	FORCEINLINE bool CanProvideSample(int32_t batchSize) const { return (m_Memory.size() >= batchSize); }
	FORCEINLINE uint64_t GetMemorySize() const { return m_Memory.size(); }

private:
	uint64_t m_Capacity = 0;
	uint64_t m_PushCount = 0;
	int32_t m_NumStates = 0;

	torch::Device m_Device;

	std::vector<Experience> m_Memory;
	std::mt19937 m_Generator;
	std::vector<int64> m_RandomNumList;

	uint64_t m_SampleCount = 0;
};

