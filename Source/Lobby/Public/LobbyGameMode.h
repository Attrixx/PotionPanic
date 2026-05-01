// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/OnlineReplStructs.h"
#include "LobbyGameState.h"
#include "LobbyGameMode.generated.h"

DEFINE_LOG_CATEGORY_STATIC(MS_LobbyGameMode, Log, All);

class ALobbyPlayerState;

USTRUCT(BlueprintType)
struct FTeleportPointsForArea
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraPosition Area{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> TeleportPoints{};
};

UCLASS()
class LOBBY_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALobbyGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:

	virtual void BeginPlay() override;

public:
	
	bool CanHandleNewPlayer() const;

	UFUNCTION(BlueprintCallable)
	void RegisterLobbySpawnPoints(const TArray<AActor*>& SpawnPoints);
	UFUNCTION(BlueprintCallable)
	void RegisterAreaTeleportPoints(const TArray<FTeleportPointsForArea>& TeleportPoints);

	UFUNCTION(BlueprintCallable)
	void RequestLeaveInviteArea(APlayerController* PlayerController);

	void OnPlayerEnterArea(ACharacter* PlayerCharacter, ECameraPosition TargetArea);
	void OnPlayerLeaveArea(ACharacter* PlayerCharacter, ECameraPosition TargetArea, bool bIsAnyActorInArea = false, bool bIsPlayerInArea = false);
	void OnPlayerEndedStartupSequence(ACharacter* PlayerCharacter);

private:

	void SpawnLobbyCharacter(APlayerController* NewPlayer);
	bool HandlePlayerNaming(APlayerController* NewPlayer, ALobbyPlayerState* PlayerState);
	bool ArePlayersOnSameConnection(APlayerController* A, APlayerController* B);
	UFUNCTION()
	void OnPreviewActorDestroyed(AActor* DestroyedActor);
	void RearrangePlayers();

	bool IsHost(AActor* Actor) const;
	void SwitchCameraForAllPlayers(ECameraPosition NewCameraPosition);
	void TeleportPlayersToArea(ECameraPosition CameraPosition);

private:

	int32 MaxPlayer = 4;
	int32 PlayerCount = 0;

	bool bHostPlayerIdInitialized = false;
	int32 HostPlayerId;

	UPROPERTY()
	TArray<AActor*> CachedSpawnPoints;

	ECameraPosition CurrentCameraPosition = ECameraPosition::Exterior;
	bool bDoorsOpen = false;

	TArray<FTeleportPointsForArea> RegisteredTeleportPoints;

};