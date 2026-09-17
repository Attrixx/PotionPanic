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

public:

	/** Where the party goes back to from the end screen. */
	const TSoftObjectPtr<UWorld>& GetLobbyLevel() const { return LobbyLevel; }

private:

	TSoftObjectPtr<UWorldData> GetWorldDataForCurrentWorld();

protected:

	UPROPERTY(EditAnywhere)
	TMap<TSoftObjectPtr<UWorld>, TSoftObjectPtr<UWorldData>> WorldsData;

	/** Lobby the end screen's "return to lobby" travels the whole party to. */
	UPROPERTY(EditAnywhere, Category = "Level")
	TSoftObjectPtr<UWorld> LobbyLevel;
};
