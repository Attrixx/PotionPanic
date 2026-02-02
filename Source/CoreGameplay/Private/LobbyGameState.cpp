// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "GameFramework/PlayerState.h"

bool ALobbyGameState::AreAllPlayersReady() const
{
	if (PlayerArray.Num() == 0) return false;

	for (APlayerState* PS : PlayerArray)
	{
		ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS);
		if (!LobbyPS || !LobbyPS->IsReady())
		{
			return false;
		}
	}
	return true;
}

void ALobbyGameState::NotifyPlayerStateChange()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ALobbyGameState::NotifyPlayerStateChange called."));
}

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	NotifyPlayerStateChange();
	OnPlayerAdded.Broadcast(PlayerState);
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	NotifyPlayerStateChange();
	OnPlayerRemoved.Broadcast(PlayerState);
}
