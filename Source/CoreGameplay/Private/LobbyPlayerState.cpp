// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"

#include "Net/UnrealNetwork.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, PlayerInfo);
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

void ALobbyPlayerState::Server_SetIsReady_Implementation(bool bNewReady)
{
	PlayerInfo.bIsReady = bNewReady;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::Client_PlayLevelSequence_Implementation(ECameraPosition TargetCameraPosition)
{
	if (CurrentCameraPosition == TargetCameraPosition) return;

	if (LobbyInterfaceWidgetInstance)
	{
		LobbyInterfaceWidgetInstance->RemoveFromParent();
		LobbyInterfaceWidgetInstance = nullptr;
	}

	ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (!LobbyGameState) return;

	ULevelSequencePlayer* PlayedSequence = nullptr;

	switch (CurrentCameraPosition)
	{
	case ECameraPosition::Exterior:
		switch (TargetCameraPosition)
		{
		case ECameraPosition::LobbyInterface:
			PlayedSequence = PlaySequence(ELevelSequenceType::ExteriorToLobbyInterface);
			break;
		case ECameraPosition::Entrance:
			PlayedSequence = PlaySequence(ELevelSequenceType::ExteriorToEntrance);
			break;
		case ECameraPosition::Interior:
			PlayedSequence = PlaySequence(ELevelSequenceType::ExteriorToInterior);
			break;
		default:
			checkNoEntry();
		}
		break;
	case ECameraPosition::Entrance:
		switch (TargetCameraPosition)
		{
		case ECameraPosition::LobbyInterface:
			PlayedSequence = PlaySequence(ELevelSequenceType::EntranceToLobbyInterface);
			break;
		case ECameraPosition::Interior:
			PlayedSequence = PlaySequence(ELevelSequenceType::EntranceToInterior);
			break;
		case ECameraPosition::Exterior:
			PlayedSequence = PlaySequence(ELevelSequenceType::ExteriorToEntrance, false);
			break;
		default:
			checkNoEntry();
		}
		break;
	case ECameraPosition::LobbyInterface:
		switch (TargetCameraPosition)
		{
		case ECameraPosition::Entrance:
			PlayedSequence = PlaySequence(ELevelSequenceType::EntranceToLobbyInterface, false);
			break;
		case ECameraPosition::Interior:
			PlayedSequence = PlaySequence(ELevelSequenceType::InteriorToLobbyInterface, false);
			break;
		case ECameraPosition::Exterior:
			PlayedSequence = PlaySequence(ELevelSequenceType::ExteriorToLobbyInterface, false);
			break;
		default:
			checkNoEntry();
		}
		break;
	case ECameraPosition::Interior:
		switch (TargetCameraPosition)
		{
		case ECameraPosition::LobbyInterface:
			PlayedSequence = PlaySequence(ELevelSequenceType::InteriorToLobbyInterface);
			break;
		case ECameraPosition::Entrance:
			PlayedSequence = PlaySequence(ELevelSequenceType::EntranceToInterior, false);
			break;
		case ECameraPosition::Exterior:
			PlayedSequence = PlaySequence(ELevelSequenceType::ExteriorToInterior, false);
			break;
		default:
			checkNoEntry();
		}
		break;

	default:
		checkNoEntry();
	}
	
	CurrentCameraPosition = TargetCameraPosition;

	if (PlayedSequence)
	{
		PlayedSequence->OnFinished.RemoveAll(this);

		if (TargetCameraPosition == ECameraPosition::LobbyInterface)
		{
			PlayedSequence->OnFinished.AddDynamic(this, &ALobbyPlayerState::OnLobbyInterfaceSequenceFinished);
		}
	}
}

ULevelSequencePlayer* ALobbyPlayerState::PlaySequence(ELevelSequenceType SequenceType, bool bPlayForward)
{
	ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (!IsValid(LobbyGameState)) return nullptr;
	ALevelSequenceActor* SequenceActor = LobbyGameState->GetLevelSequenceActor(SequenceType);
	if (!IsValid(SequenceActor)) return nullptr;
	ULevelSequencePlayer* SequencePlayer = SequenceActor->GetSequencePlayer();
	if (!IsValid(SequencePlayer)) return nullptr;
	if (bPlayForward)
	{
		SequencePlayer->Play();
	}
	else
	{
		SequencePlayer->PlayReverse();
	}
	return SequencePlayer;
}

void ALobbyPlayerState::Server_OnStartupSequenceFinished_Implementation()
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->OnPlayerEndedStartupSequence(Cast<ACharacter>(GetPawn()));
	}
}

void ALobbyPlayerState::OnLobbyInterfaceSequenceFinished()
{
	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController() && LobbyInterfaceWidgetClass)
	{
		if (!LobbyInterfaceWidgetInstance)
		{
			LobbyInterfaceWidgetInstance = CreateWidget<UUserWidget>(PC, LobbyInterfaceWidgetClass);
			if (LobbyInterfaceWidgetInstance)
			{
				LobbyInterfaceWidgetInstance->AddToViewport();
			}
		}
	}
}

void ALobbyPlayerState::OnRep_PlayerInfo()
{
	OnPlayerInfoChanged.Broadcast();
	OnPlayerColorChanged.Broadcast(PlayerInfo.PlayerColor);
}
