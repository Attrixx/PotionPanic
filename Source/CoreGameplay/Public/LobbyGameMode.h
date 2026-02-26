// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/OnlineReplStructs.h"
#include "LobbyGameState.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlayerPreview;
class ALobbySpawnPoint;
class ALobbyPlayerState;
class ATriggerBox;

USTRUCT(BlueprintType)
struct FTeleportPointsForArea
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraPosition Area;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> TeleportPoints;
};

UCLASS()
class COREGAMEPLAY_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALobbyGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:

	virtual void BeginPlay() override;

	
	void OnNewPlayerLogin(int32 PlayerId, const FString& PlayerName, bool bIsHost);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<ALobbyPlayerPreview> LobbyPlayerPreviewClass;

public:
	
	bool CanHandleNewPlayer();

	UFUNCTION(BlueprintCallable)
	void RegisterTriggerBoxes(const TMap<ETriggerBoxType, ATriggerBox*>& TriggerBoxes);
	UFUNCTION(BlueprintCallable)
	void RegisterLobbySpawnPoints(const TArray<ALobbySpawnPoint*>& SpawnPoints);
	UFUNCTION(BlueprintCallable)
	void RegisterAreaTeleportPoints(const TArray<FTeleportPointsForArea>& TeleportPoints);

	UFUNCTION(BlueprintCallable)
	void RequestLeaveInviteArea(APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable)
	void TMP_TravelToLevel(const FString& LevelUrl);

private:

	void SpawnLobbyCharacter(APlayerController* NewPlayer);
	bool HandlePlayerNaming(APlayerController* NewPlayer, ALobbyPlayerState* PlayerState);
	bool ArePlayersOnSameConnection(APlayerController* A, APlayerController* B);
	void RearrangePlayers();

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(AActor* TriggerBox, AActor* OtherActor);
	UFUNCTION()
	void OnTriggerBoxEndOverlap(AActor* TriggerBox, AActor* OtherActor);

	bool IsHost(AActor* Actor) const;
	void SwitchCameraForAllPlayers(ECameraPosition NewCameraPosition);
	void SwitchCameraForTarget(AActor* TargetActor, ECameraPosition NewCameraPosition);
	bool IsAnyActorInTriggerBox(TSubclassOf<AActor> ClassToSearch, ETriggerBoxType TriggerBoxType) const;
	bool IsActorInTriggerBox(AActor* Actor, ETriggerBoxType TriggerBoxType) const;
	void TeleportPlayersToArea(ECameraPosition CameraPosition);

private:

	int32 MaxPlayer = 4;
	int32 PlayerCount = 0;

	bool bHostPlayerIdInitialized = false;
	int32 HostPlayerId;

	UPROPERTY()
	TArray<ALobbySpawnPoint*> CachedSpawnPoints;

	TMap<ETriggerBoxType, ATriggerBox*> RegisteredTriggerBoxes;
	ECameraPosition CurrentCameraPosition = ECameraPosition::Exterior;
	bool bDoorsOpen = false;

	TArray<FTeleportPointsForArea> RegisteredTeleportPoints;

};