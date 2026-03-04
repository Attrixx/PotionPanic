// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"

#include "Net/UnrealNetwork.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"

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

void ALobbyPlayerState::PlayLevelSequence(ECameraPosition TargetCameraPosition)
{
	ClientPlayLevelSequence(TargetCameraPosition);
}

void ALobbyPlayerState::ClientPlayLevelSequence_Implementation(ECameraPosition TargetCameraPosition)
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
		}
		break;
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

void ALobbyPlayerState::ServerOnStartupSequenceFinished_Implementation()
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
