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

	DOREPLIFETIME(ALobbyGameState, LobbyCamera);
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

void ALobbyGameState::MulticastPlayLevelSequence_Implementation(ECameraPosition TargetCameraPosition)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Iterator->Get())
		{
			if (PC->IsLocalController())
			{
				if (ALobbyPlayerState* PS = PC->GetPlayerState<ALobbyPlayerState>())
				{
					PS->PlayLevelSequence(TargetCameraPosition);
				}
			}
		}
	}
}

void ALobbyGameState::MulticastOpenDoors_Implementation(bool bOpen)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Iterator->Get())
		{
			if (PC->IsLocalController())
			{
				if (ALobbyPlayerState* PS = PC->GetPlayerState<ALobbyPlayerState>())
				{
					PS->PlaySequence(ELevelSequenceType::OpenDoors, bOpen);
				}
			}
		}
	}
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
