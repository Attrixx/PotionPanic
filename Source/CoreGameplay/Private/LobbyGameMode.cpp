// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "LobbyCharacter.h"
#include "LobbyPlayerController.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/Character.h"
#include "Online/CoreOnline.h"
#include "OnlineSubsystemTypes.h"
#include "Kismet/GameplayStatics.h"

ALobbyGameMode::ALobbyGameMode()
{
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	if (!CanHandleNewPlayer())
	{
		ErrorMessage = TEXT("The lobby is full");
		return;
	}
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerCount++;
	UE_LOG(MS_LobbyGameMode, Log, TEXT("New player connected. Total: %d"), PlayerCount);

	SpawnLobbyCharacter(NewPlayer);

	if (ALobbyPlayerState* PlayerState = NewPlayer->GetPlayerState<ALobbyPlayerState>())
	{
		bool bIsSibling = HandlePlayerNaming(NewPlayer, PlayerState);

		bool bIsHost = NewPlayer->HasAuthority() && NewPlayer->IsLocalController() && !bIsSibling;

		if (bIsHost)
		{
			bHostPlayerIdInitialized = true;
			HostPlayerId = PlayerState->GetPlayerId();
			PlayerState->Server_SetIsHost(true);
		}
	}

	if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(NewPlayer))
	{
		PC->ClientSwitchMappingContext(CurrentCameraPosition == ECameraPosition::LobbyInterface);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (ALobbyPlayerController* LeavingPC = Cast<ALobbyPlayerController>(Exiting))
	{
		if (ALobbyCharacter* PreviewActor = LeavingPC->GetPreviewActor())
		{
			PreviewActor->OnDestroyed.AddDynamic(this, &ALobbyGameMode::OnPreviewActorDestroyed);
			PreviewActor->Destroy();
			PreviewActor = nullptr;
		}
	}

	PlayerCount = FMath::Max(0, PlayerCount - 1);
	UE_LOGFMT(MS_LobbyGameMode, Log, "Player left. Remaining players: {Count}", PlayerCount);

	Super::Logout(Exiting);
}

void ALobbyGameMode::SpawnLobbyCharacter(APlayerController* NewPlayer)
{
	if (!NewPlayer || CachedSpawnPoints.Num() < 1) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 NumberOfPlayers = GetNumPlayers();
	if (CachedSpawnPoints.Num() >= NumberOfPlayers)
	{
		AActor* ChosenPoint = CachedSpawnPoints[NumberOfPlayers - 1];
		ALobbyCharacter* NewPreview = GetWorld()->SpawnActor<ALobbyCharacter>(
			DefaultPawnClass,
			ChosenPoint->GetActorLocation(),
			ChosenPoint->GetActorRotation(),
			SpawnParams
		);

		if (ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(NewPlayer))
		{
			LobbyPlayerController->SetPreviewActor(NewPreview);
			if (ALobbyPlayerState* LobbyPlayerState = LobbyPlayerController->GetPlayerState<ALobbyPlayerState>())
			{
				LobbyPlayerState->OnPlayerColorChanged.AddDynamic(NewPreview, &ALobbyCharacter::SetPlayerColor);
				NewPreview->SetPlayerColor(LobbyPlayerState->GetPlayerColor());
			}

			if (CurrentCameraPosition != ECameraPosition::LobbyInterface)
			{
				NewPreview->SetActorHiddenInGame(true);
			}
		}
	}
}

bool ALobbyGameMode::ArePlayersOnSameConnection(APlayerController* A, APlayerController* B)
{
	if (!A || !B)
	{
		return false;
	}

	UNetConnection* NetConnA = A->GetNetConnection();
	UNetConnection* NetConnB = B->GetNetConnection();

	if (NetConnA && NetConnB)
	{
		return NetConnA->GetConnectionHandle().GetParentConnectionId() == NetConnB->GetConnectionHandle().GetParentConnectionId();
	}
	else if (!NetConnA && !NetConnB)
	{
		return A->IsLocalController() && B->IsLocalController();
	}
	return false;
}

void ALobbyGameMode::OnPreviewActorDestroyed(AActor* DestroyedActor)
{
	RearrangePlayers();
}

void ALobbyGameMode::RearrangePlayers()
{
	int32 PlayerIndex = 0;
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get()))
		{
			if (CachedSpawnPoints.IsValidIndex(PlayerIndex))
			{
				ALobbyCharacter* PreviewActor = PC->GetPreviewActor();
				if (!IsValid(PreviewActor) || PreviewActor->IsActorBeingDestroyed()) continue;
				
				if (CachedSpawnPoints.IsValidIndex(PlayerIndex))
				{
					PreviewActor->SetActorLocation(CachedSpawnPoints[PlayerIndex]->GetActorLocation());
				}
			}
		}
		PlayerIndex++;
	}
}

bool ALobbyGameMode::IsHost(AActor* Actor) const
{
	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		if (ALobbyPlayerState* PS = Character->GetPlayerState<ALobbyPlayerState>())
		{
			return PS->IsHost();
		}
	}
	return false;
}

void ALobbyGameMode::SwitchCameraForAllPlayers(ECameraPosition NewCameraPosition)
{
	CurrentCameraPosition = NewCameraPosition;
	if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
	{
		LobbyGameState->MulticastPlayLevelSequence(NewCameraPosition);
	}
}

void ALobbyGameMode::SwitchCameraForTarget(AActor* TargetActor, ECameraPosition NewCameraPosition)
{
	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		Character->GetPlayerState<ALobbyPlayerState>()->PlayLevelSequence(NewCameraPosition);
	}
}

void ALobbyGameMode::TeleportPlayersToArea(ECameraPosition CameraPosition)
{
	TArray<AActor*> TeleportPoints;
	for (const FTeleportPointsForArea& AreaPoints : RegisteredTeleportPoints)
	{
		if (AreaPoints.Area == CameraPosition)
		{
			TeleportPoints = AreaPoints.TeleportPoints;
			break;
		}
	}

	int32 CurrentIndex = 0;
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACharacter* Target = (*It)->GetCharacter();
		if (IsValid(Target) && !IsHost(Target) && CurrentIndex < TeleportPoints.Num() - 1)
		{
			Target->SetActorLocation(TeleportPoints[CurrentIndex]->GetActorLocation());
			Target->SetActorRotation(TeleportPoints[CurrentIndex]->GetActorRotation());
		}
	}
}

bool ALobbyGameMode::HandlePlayerNaming(APlayerController* NewPlayer, ALobbyPlayerState* PlayerState)
{
	FString PlayerName = PlayerState->GetPlayerName();
	FString BaseName = PlayerName;
	TArray<int32> UsedIndices;
	bool bFoundSiblings = false;

	bool bIsChildConnection = NewPlayer->GetNetConnection() == nullptr || NewPlayer->GetNetConnection()->GetConnectionHandle().IsChildConnection();
	if (!bIsChildConnection)
	{
		return false;
	}

	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* OtherPC = It->Get();
		if (!OtherPC || OtherPC == NewPlayer) continue;

		if (ArePlayersOnSameConnection(NewPlayer, OtherPC))
		{
			bFoundSiblings = true;
			if (APlayerState* OtherPS = OtherPC->GetPlayerState<APlayerState>())
			{
				FString OtherName = OtherPS->GetPlayerName();
				int32 OtherIndex = 0;
				int32 OpenParenIdx;

				if (OtherName.FindLastChar('(', OpenParenIdx) && OtherName.EndsWith(TEXT(")")))
				{
					FString IndexStr = OtherName.Mid(OpenParenIdx + 1, OtherName.Len() - OpenParenIdx - 2);
					if (IndexStr.IsNumeric())
					{
						OtherIndex = FCString::Atoi(*IndexStr);
						BaseName = OtherName.Left(OpenParenIdx);
					}
				}
				else
				{
					BaseName = OtherName;
				}

				UsedIndices.Add(OtherIndex);
			}
		}
	}

	if (bFoundSiblings)
	{
		int32 NewIndex = 1;
		while (UsedIndices.Contains(NewIndex))
		{
			NewIndex++;
		}

		PlayerState->SetPlayerName(FString::Printf(TEXT("%s(%d)"), *BaseName, NewIndex));
	}

	return bFoundSiblings;
}

bool ALobbyGameMode::CanHandleNewPlayer() const
{
	if (PlayerCount >= MaxPlayer)
	{
		UE_LOG(MS_LobbyGameMode, Warning, TEXT("Connection refused: lobby full"));
		return false;
	}
	return true;
}

void ALobbyGameMode::RegisterLobbySpawnPoints(const TArray<AActor*>& SpawnPoints)
{
	CachedSpawnPoints = SpawnPoints;

	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get()))
		{
			if (!PC->GetPreviewActor())
			{
				SpawnLobbyCharacter(PC);
			}
		}
	}

	RearrangePlayers();
}

void ALobbyGameMode::RegisterAreaTeleportPoints(const TArray<FTeleportPointsForArea>& TeleportPoints)
{
	RegisteredTeleportPoints = TeleportPoints;
}

void ALobbyGameMode::RequestLeaveInviteArea(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) && PlayerController->GetPlayerState<ALobbyPlayerState>()->IsHost())
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(Iterator->Get()))
			{
				PC->ClientSwitchMappingContext(false);
			}
		}
	}
	SwitchCameraForAllPlayers(ECameraPosition::Exterior);
}

void ALobbyGameMode::OnPlayerEnterArea(ACharacter* PlayerCharacter, ECameraPosition TargetArea)
{
	switch (TargetArea)
	{
	case ECameraPosition::LobbyInterface:
		if (!IsHost(PlayerCharacter)) return;
		RearrangePlayers();
		SwitchCameraForAllPlayers(ECameraPosition::LobbyInterface);
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(Iterator->Get()))
			{
				PC->ClientSwitchMappingContext(true);
			}
		}
		break;
	case ECameraPosition::Entrance:
		SwitchCameraForTarget(PlayerCharacter, ECameraPosition::Entrance);
		if (bDoorsOpen) return;
		bDoorsOpen = true;
		if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
		{
			LobbyGameState->MulticastOpenDoors();
		}
		break;
	case ECameraPosition::Interior:
		if (IsHost(PlayerCharacter))
		{
			SwitchCameraForAllPlayers(ECameraPosition::Interior);
			TeleportPlayersToArea(ECameraPosition::Interior);
		}
		else
		{
			SwitchCameraForTarget(PlayerCharacter, ECameraPosition::Interior);
		}
		break;
	}
}

void ALobbyGameMode::OnPlayerLeaveArea(ACharacter* PlayerCharacter, ECameraPosition TargetArea, bool bIsAnyActorInArea, bool bIsPlayerInArea)
{
	switch (TargetArea)
	{
	case ECameraPosition::LobbyInterface:
		break;
	case ECameraPosition::Entrance:
		if (bDoorsOpen && !bIsAnyActorInArea)
		{
			bDoorsOpen = false;
			if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
			{
				LobbyGameState->MulticastOpenDoors(false);
			}
		}

		if (IsHost(PlayerCharacter))
		{
			if (CurrentCameraPosition == ECameraPosition::Interior)
			{
				SwitchCameraForAllPlayers(ECameraPosition::Exterior);
				TeleportPlayersToArea(ECameraPosition::Exterior);
			}
		}
		else
		{
			if (!bIsPlayerInArea)
			{
				SwitchCameraForTarget(PlayerCharacter, ECameraPosition::Exterior);
			}
		}
		break;
	case ECameraPosition::Interior:
		break;
	}
}

void ALobbyGameMode::OnPlayerEndedStartupSequence(ACharacter* PlayerCharacter)
{
	if (CurrentCameraPosition == ECameraPosition::LobbyInterface)
	{
		SwitchCameraForTarget(PlayerCharacter, ECameraPosition::LobbyInterface);
	}
}
