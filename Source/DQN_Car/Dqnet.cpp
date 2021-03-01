// Fill out your copyright notice in the Description page of Project Settings.


#include "Dqnet.h"

constexpr int32_t g_HiddenLayer1 = 32;
constexpr int32_t g_HiddenLayer2 = 16;
constexpr int32_t g_HiddenLayer3 = 8;

DqnetImpl::DqnetImpl(int32_t numStates, int32_t numActions)
	:m_Linear1(torch::nn::LinearOptions(numStates, g_HiddenLayer1)),
	m_Linear2(torch::nn::LinearOptions(g_HiddenLayer1, g_HiddenLayer2)),
	m_Linear3(torch::nn::LinearOptions(g_HiddenLayer2, g_HiddenLayer3)),
	m_Out(torch::nn::LinearOptions(g_HiddenLayer3, numActions))
{
	register_module("m_Linear1", m_Linear1);
	register_module("m_Linear2", m_Linear2);
	register_module("m_Linear3", m_Linear3);
	register_module("m_Out", m_Out);
}

DqnetImpl::~DqnetImpl()
{
}

torch::Tensor DqnetImpl::forward(torch::Tensor x)
{
	x = torch::relu(m_Linear1(x));
	x = torch::relu(m_Linear2(x));
	x = torch::relu(m_Linear3(x));
	x = m_Out(x);
	return x;
}

