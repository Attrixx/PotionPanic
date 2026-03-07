// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AlchemyGameState.generated.h"

class AAlchemyWorldSettings;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API AAlchemyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	void BeginPlay() override;
	
private:
	
	void InitializeSubsystems(AAlchemyWorldSettings& WorldSettings);
};
