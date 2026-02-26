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

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ALobbyPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	void BeginPlay() override;
	void OnRep_PlayerName() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetPlayerColor(FColor NewColor);

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerColor(FColor NewColor);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FColor GetPlayerColor() const { return PlayerColor; }

	UFUNCTION(Server, Reliable)
	void Server_SetIsHost(bool bHost);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetIsReady(bool bReady);

	UFUNCTION(Server, Reliable)
	void Server_SetIsReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsHost() const { return bIsHost; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsCouchCoopPlayer() const { return bIsCouchCoopPlayer; }

	void PlayLevelSequence(ECameraPosition TargetCameraPosition);
	UFUNCTION(Client, Reliable)
	void ClientPlayLevelSequence(ECameraPosition TargetCameraPosition);
	ULevelSequencePlayer* PlaySequence(ELevelSequenceType SequenceType, bool bPlayForward = true);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Lobby|UI")
	TSubclassOf<UUserWidget> LobbyInterfaceWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyInterfaceWidgetInstance;

	UFUNCTION()
	void OnLobbyInterfaceSequenceFinished();

	UPROPERTY(ReplicatedUsing = OnRep_IsHost, VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bIsHost;

	UFUNCTION()
	void OnRep_IsHost();

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor, VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	FColor PlayerColor;

	UFUNCTION()
	void OnRep_PlayerColor();

	UPROPERTY(ReplicatedUsing = OnRep_IsReady, VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bIsReady;

	UFUNCTION()
	void OnRep_IsReady();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bIsCouchCoopPlayer;

	ECameraPosition CurrentCameraPosition = ECameraPosition::Exterior;

public:
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerStateUpdated OnPlayerInfoChanged;
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerColorChanged OnPlayerColorChanged;
};
