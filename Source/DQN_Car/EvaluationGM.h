// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EvaluationGM.generated.h"

/**
 * 
 */
UCLASS()
class DQN_CAR_API AEvaluationGM : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AEvaluationGM();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
