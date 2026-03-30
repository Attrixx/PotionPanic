// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyGameState.h"
#include "LobbyPlayerState.generated.h"

class UUserWidget;
class ULevelSequencePlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayerStateUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerColorChanged, FColor, NewColor);


USTRUCT(BlueprintType)
struct FLobbyPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor PlayerColor = FColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCouchCoopPlayer = false;
};


/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	void BeginPlay() override;
	void OnRep_PlayerName() override;

public:

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerColor(FColor NewColor);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FColor GetPlayerColor() const { return PlayerInfo.PlayerColor; }

	// Not an RPC, only server should be allowed to set this
	void SetIsHost(bool bHost);

	UFUNCTION(Server, Reliable)
	void Server_SetIsReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsReady() const { return PlayerInfo.bIsReady; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsHost() const { return PlayerInfo.bIsHost; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsCouchCoopPlayer() const { return PlayerInfo.bIsCouchCoopPlayer; }

	UFUNCTION(Client, Reliable)
	void Client_PlayLevelSequence(ECameraPosition TargetCameraPosition);
	
	ULevelSequencePlayer* PlaySequence(ELevelSequenceType SequenceType, bool bPlayForward = true);

	UFUNCTION(Server, Reliable)
	void Server_OnStartupSequenceFinished();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Lobby|UI")
	TSubclassOf<UUserWidget> LobbyInterfaceWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyInterfaceWidgetInstance;

	UFUNCTION()
	void OnLobbyInterfaceSequenceFinished();

	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FLobbyPlayerInfo PlayerInfo;

	UFUNCTION()
	void OnRep_PlayerInfo();


	ECameraPosition CurrentCameraPosition = ECameraPosition::Exterior;

public:
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerStateUpdated OnPlayerInfoChanged;
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerColorChanged OnPlayerColorChanged;
};
