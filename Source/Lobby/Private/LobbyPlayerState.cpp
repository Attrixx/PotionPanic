// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "AlchemistBase.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, PlayerInfo);
	DOREPLIFETIME(ALobbyPlayerState, PreviewActor);
}

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();
	OnPlayerInfoChanged.Broadcast();
	OnPawnSet.AddDynamic(this, &ALobbyPlayerState::HandlePawnSet);
}

void ALobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerInfoChanged.Broadcast();
}

void ALobbyPlayerState::Server_SetPlayerColor_Implementation(FColor NewColor)
{
	PlayerInfo.PlayerColor = NewColor;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::SetIsHost(bool bHost)
{
	PlayerInfo.bIsHost = bHost;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::SetPreviewActor(AAlchemistBase* InPreviewActor)
{
	if (HasAuthority())
	{
		PreviewActor = InPreviewActor;
		OnRep_PreviewActor();
	}
}

void ALobbyPlayerState::OnRep_PreviewActor()
{
	if (IsValid(PreviewActor))
	{
		PreviewActor->SetColor(PlayerInfo.PlayerColor);
	}
}

void ALobbyPlayerState::Server_SetIsReady_Implementation(bool bNewReady)
{
	PlayerInfo.bIsReady = bNewReady;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::Server_OnStartupSequenceFinished_Implementation()
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->OnPlayerEndedStartupSequence(Cast<ACharacter>(GetPawn()));
	}
}

void ALobbyPlayerState::OnRep_PlayerInfo()
{
	OnPlayerInfoChanged.Broadcast();
	OnPlayerColorChanged.Broadcast(PlayerInfo.PlayerColor);

	if (AAlchemistBase* Character = Cast<AAlchemistBase>(GetPawn()))
	{
		Character->SetColor(PlayerInfo.PlayerColor);
	}

	if (IsValid(PreviewActor))
	{
		PreviewActor->SetColor(PlayerInfo.PlayerColor);
	}
}

void ALobbyPlayerState::HandlePawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (AAlchemistBase* Character = Cast<AAlchemistBase>(NewPawn))
	{
		Character->SetColor(PlayerInfo.PlayerColor);
	}
}
