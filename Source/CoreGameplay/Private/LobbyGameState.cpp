// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ALobbyGameState::ServerRegisterLevelSequences_Implementation(const TArray<FLevelSequenceInfo>& LevelSequences)
{
	// Ensure that we have only one entry per ELevelSequenceType
	// If there are multiple entries for the same ELevelSequenceType, the last one will be used
	RegisteredLevelSequences.Empty();
	TMap<ELevelSequenceType, int> SequenceTypeIndexes;
	for (const FLevelSequenceInfo& Info : LevelSequences)
	{
		if (SequenceTypeIndexes.Contains(Info.SequenceType))
		{
			RegisteredLevelSequences[SequenceTypeIndexes[Info.SequenceType]] = Info;
		}
		else
		{
			RegisteredLevelSequences.Add(Info);
			SequenceTypeIndexes.Add(Info.SequenceType, RegisteredLevelSequences.Num() - 1);
		}
	}
}

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

void ALobbyGameState::SetCameraPosition(ECameraPosition NewCameraPosition)
{
	TargetCameraPosition = NewCameraPosition;
	OnRep_TargetCameraPosition();
}

void ALobbyGameState::SetDoorsOpen(bool bOpen)
{
	bDoorsOpen = bOpen;
	OnRep_DoorsOpen();
}

ALevelSequenceActor* ALobbyGameState::GetLevelSequenceActor(ELevelSequenceType SequenceType) const
{
	for (const auto& Info : RegisteredLevelSequences)
	{
		if (Info.SequenceType == SequenceType)
		{
			return Info.SequenceActor;
		}
	}
	return nullptr;
}

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPlayerAdded.Broadcast(PlayerState);
	
	if (HasAuthority())
	{
		if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PlayerState))
		{
			for (int32 i = 0; i < AvailableColors.Num(); i++)
			{
				if (AvailableColors[i])
				{
					LobbyPS->SetPlayerColor(DefaultColors[i]);
					AvailableColors[i] = false;
					return;
				}
			}

			LobbyPS->SetPlayerColor(FColor::MakeRandomColor());
		}
	}
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnPlayerRemoved.Broadcast(PlayerState);
	
	if (HasAuthority())
	{
		if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PlayerState))
		{
			int32 Index = DefaultColors.Find(LobbyPS->GetPlayerColor());
			if (Index != INDEX_NONE)
			{
				AvailableColors[Index] = true;
			}
		}
	}
}

void ALobbyGameState::OnRep_TargetCameraPosition()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (!IsValid(PC) || !PC->IsLocalController())
		{
			continue;
		}

		ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
		if (!IsValid(LocalPlayer) || LocalPlayer->GetControllerId() != 0)
		{
			continue;
		}

		ALobbyPlayerState* PS = PC->GetPlayerState<ALobbyPlayerState>();
		if (!IsValid(PS))
		{
			continue;
		}

		PS->PlayLevelSequence(TargetCameraPosition);
	}
}

void ALobbyGameState::OnRep_DoorsOpen()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Iterator->Get())
		{
			if (PC->IsLocalController())
			{
				if (ALobbyPlayerState* PS = PC->GetPlayerState<ALobbyPlayerState>())
				{
					PS->PlaySequence(ELevelSequenceType::OpenDoors, bDoorsOpen);
				}
			}
		}
	}
}
