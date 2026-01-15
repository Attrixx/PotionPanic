// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALobbyGameMode();

protected:

	void BeginPlay() override;

public:

	bool CanHandleNewPlayer();

private:

	int32 PlayerCount = 0;
	
};
