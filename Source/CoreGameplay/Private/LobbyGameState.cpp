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

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPlayerAdded.Broadcast(PlayerState);
	if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PlayerState))
	{
		if (AvailableDefaultColors.Num() > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Assigning default color %s to player %s"), *AvailableDefaultColors[0].ToString(), *LobbyPS->GetPlayerName()));
			LobbyPS->SetPlayerColor(AvailableDefaultColors[0]);
			AvailableDefaultColors.RemoveAt(0);
		}
		else
		{
			LobbyPS->SetPlayerColor(FColor::MakeRandomColor());
		}
	}
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnPlayerRemoved.Broadcast(PlayerState);
	if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PlayerState))
	{
		AvailableDefaultColors.Add(LobbyPS->GetPlayerColor());
	}
}
