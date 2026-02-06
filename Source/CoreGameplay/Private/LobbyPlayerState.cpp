// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"

#include "Net/UnrealNetwork.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	PlayerColor = FColor::Green;
	bIsReady = false;
	bIsHost = false;
	bIsCouchCoopPlayer = false;
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, PlayerColor);
	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
	DOREPLIFETIME(ALobbyPlayerState, bIsHost);
	DOREPLIFETIME(ALobbyPlayerState, bIsCouchCoopPlayer);
}

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();
	OnPlayerInfoChanged.Broadcast();
}

void ALobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerInfoChanged.Broadcast();
}

void ALobbyPlayerState::SetPlayerColor(FColor NewColor)
{
	if (HasAuthority())
	{
		PlayerColor = NewColor;
		OnRep_PlayerColor();
	}
	else
	{
		Server_SetPlayerColor(NewColor);
	}
}

void ALobbyPlayerState::Server_SetPlayerColor_Implementation(FColor NewColor)
{
	SetPlayerColor(NewColor);
}

void ALobbyPlayerState::Server_SetIsHost_Implementation(bool bHost)
{
	bIsHost = bHost;
	OnRep_IsHost();
}

void ALobbyPlayerState::SetIsReady(bool bNewReady)
{
	if (HasAuthority())
	{
		bIsReady = bNewReady;
		OnRep_IsReady();
	}
	else
	{
		Server_SetIsReady(bNewReady);
	}
}

void ALobbyPlayerState::Server_SetIsReady_Implementation(bool bNewReady)
{
	SetIsReady(bNewReady);
}

void ALobbyPlayerState::OnRep_IsHost()
{
	OnPlayerInfoChanged.Broadcast();
}

void ALobbyPlayerState::OnRep_PlayerColor()
{
	OnPlayerInfoChanged.Broadcast();
	OnPlayerColorChanged.Broadcast(PlayerColor);
}

void ALobbyPlayerState::OnRep_IsReady()
{
	OnPlayerInfoChanged.Broadcast();
}
