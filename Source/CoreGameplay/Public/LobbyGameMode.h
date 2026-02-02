// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFrameWork/OnlineReplStructs.h"
#include "LobbyGameMode.generated.h"


UCLASS()
class COREGAMEPLAY_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALobbyGameMode();
	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;
	void PreLogin(const FString& Options, const FString& Adress, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

protected:

	void BeginPlay() override;

	void OnNewPlayerLogin(int32 PlayerId, const FString& PlayerName, bool bIsHost);

public:

	bool CanHandleNewPlayer();

private:

	int32 MaxPlayer = 4; 
	int32 PlayerCount = 0;

	bool bHostPlayerIdInitialized = false;
	int32 HostPlayerId;

public:
	void CheckGameStart();
	void StartGame();
	
};
