// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerAdded, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerRemoved, APlayerState*, PlayerState);

UCLASS()
class COREGAMEPLAY_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool AreAllPlayersReady() const;

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerAdded OnPlayerAdded;

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerRemoved OnPlayerRemoved;

protected:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

private:

	TArray<FColor> AvailableDefaultColors = {
		FColor::Red,
		FColor::Green,
		FColor::Blue,
		FColor::Yellow
	};

};
