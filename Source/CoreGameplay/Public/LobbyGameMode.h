// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/OnlineReplStructs.h" // Correction de la faute de frappe "GameFrameWork" -> "GameFramework"
#include "LobbyGameMode.generated.h"

// Forward Declarations pour éviter les dépendances circulaires
class ALobbyPlayerPreview;
class ALobbySpawnPoint;
class ALobbyPlayerState;

UCLASS()
class COREGAMEPLAY_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();


	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

protected:
	virtual void BeginPlay() override;

	
	void OnNewPlayerLogin(int32 PlayerId, const FString& PlayerName, bool bIsHost);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<ALobbyPlayerPreview> LobbyPlayerPreviewClass;

public:
	
	bool CanHandleNewPlayer();
	void CheckGameStart();
	void StartGame();

private:

	ALobbySpawnPoint* FindFreeSpawnPoint();
	void SpawnLobbyCharacter(APlayerController* NewPlayer, ALobbySpawnPoint* ChosenPoint);
	bool HandlePlayerNaming(APlayerController* NewPlayer, ALobbyPlayerState* PlayerState);
	bool ArePlayersOnSameConnection(APlayerController* A, APlayerController* B);

private:
	int32 MaxPlayer = 4;
	int32 PlayerCount = 0;

	bool bHostPlayerIdInitialized = false;
	int32 HostPlayerId;

	
	UPROPERTY()
	TArray<ALobbySpawnPoint*> CachedSpawnPoints;
};