// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, GlobalArea);
	DOREPLIFETIME(ALobbyGameState, bDoorsOpen);
	DOREPLIFETIME(ALobbyGameState, RegisteredLevelSequences);
}

void ALobbyGameState::ServerRegisterLevelSequences_Implementation(const TArray<FLevelSequenceInfo>& LevelSequences)
{
	RegisteredLevelSequences = LevelSequences;
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

void ALobbyGameState::SetGlobalArea(ECameraPosition NewCameraPosition)
{
	if (HasAuthority())
	{
		GlobalArea = NewCameraPosition;
		OnRep_GlobalArea();
	}
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
					LobbyPS->SetPlayerIndex(i);
					LobbyPS->Server_SetPlayerColor(DefaultColors[i]);
					AvailableColors[i] = false;
					return;
				}
			}

			LobbyPS->Server_SetPlayerColor(EAlchemistColor(FMath::RandRange(0, (int32)EAlchemistColor::Count - 1)));
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
			int32 PlayerIndex = LobbyPS->GetPlayerIndex();
			if (AvailableColors.IsValidIndex(PlayerIndex))
			{
				AvailableColors[PlayerIndex] = true;
			}
		}
	}
}

void ALobbyGameState::UpdatePlayerColorInMPC(int32 PlayerIndex, EAlchemistColor NewColor)
{
	if (!IsValid(PlayerColorMPC) || !IsValid(CustomizationData)) return;

	UMaterialParameterCollectionInstance* MPCI = GetWorld()->GetParameterCollectionInstance(PlayerColorMPC);
	if (!IsValid(MPCI)) return;

	FName ParameterName = FName(*FString::Printf(TEXT("Player%d_Color"), PlayerIndex + 1));
	MPCI->SetVectorParameterValue(ParameterName, CustomizationData->GetLinearColor(NewColor));
}

void ALobbyGameState::OnRep_GlobalArea()
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

		if (ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(PC))
		{
			LobbyPC->TransitionToArea(GlobalArea);
		}
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
				if (ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(PC))
				{
					LobbyPC->PlaySequence(ELevelSequenceType::OpenDoors, bDoorsOpen);
				}
			}
		}
	}
}
