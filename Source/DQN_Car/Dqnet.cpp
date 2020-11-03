// Fill out your copyright notice in the Description page of Project Settings.


#include "Dqnet.h"

const int32_t g_HiddenLayer1 = 16;
const int32_t g_HiddenLayer2 = 8;

DqnetImpl::DqnetImpl(int32_t numStates, int32_t numActions)
	:m_Linear1(torch::nn::LinearOptions(numStates, g_HiddenLayer1)),
	m_Linear2(torch::nn::LinearOptions(g_HiddenLayer1, g_HiddenLayer2)),
	m_Out(torch::nn::LinearOptions(g_HiddenLayer2, numActions))
{
	register_module("m_Linear1", m_Linear1);
	register_module("m_Linear2", m_Linear2);
	register_module("m_Out", m_Out);
}

DqnetImpl::~DqnetImpl()
{
}

torch::Tensor DqnetImpl::forward(torch::Tensor x)
{
	x = torch::relu(m_Linear1(x));
	x = torch::relu(m_Linear2(x));
	x = m_Out(x);
	return x;
}

