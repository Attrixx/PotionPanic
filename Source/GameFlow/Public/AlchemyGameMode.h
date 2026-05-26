// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AlchemyGameMode.generated.h"

class UWorldData;

/**
 * 
 */
UCLASS(Abstract)
class GAMEFLOW_API AAlchemyGameMode : public AGameModeBase
{
	GENERATED_BODY()

	AAlchemyGameMode();
	void InitGameState() override;
	
private:
	
	TSoftObjectPtr<UWorldData> GetWorldDataForCurrentWorld();

protected:
	
	UPROPERTY(EditAnywhere)
	TMap<TSoftObjectPtr<UWorld>, TSoftObjectPtr<UWorldData>> WorldsData;
};
