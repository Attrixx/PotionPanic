// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Online/CoreOnline.h"
#include "OnlineSubsystemTypes.h"
#include "GameFramework/OnlineReplStructs.h"

#include "LobbySpawnPoint.h"
#include "LobbyPlayerPreview.h"
#include "LobbyPlayerController.h"
#include "LobbyPlayerState.h"
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
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!CanHandleNewPlayer())
	{
		ErrorMessage = TEXT("The lobby is full");
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerCount++;
	UE_LOG(LogTemp, Log, TEXT("PostLogin: New player connected. Total: %d"), PlayerCount);

	CacheSpawnPoints();
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

		const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
		if (UniqueId.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("PostLogin Info: Name: %s, ID: %d, UniqueID: %s"),
				*PlayerState->GetPlayerName(),
				PlayerState->GetPlayerId(),
				*UniqueId.GetV1()->ToDebugString());
		}

		OnNewPlayerLogin(PlayerState->GetPlayerId(), PlayerState->GetPlayerName(), bIsHost);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (ALobbyPlayerController* LeavingPC = Cast<ALobbyPlayerController>(Exiting))
	{
		if (LeavingPC->MyPreviewActor)
		{
			LeavingPC->MyPreviewActor->Destroy();
			LeavingPC->MyPreviewActor = nullptr;
		}
	}

	PlayerCount = FMath::Max(0, PlayerCount - 1);
	UE_LOG(LogTemp, Log, TEXT("Player left. Remaining players: %d"), PlayerCount);

	Super::Logout(Exiting);

	// Add a delay before rearranging players to ensure the leaving player's preview is fully removed from the world
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ALobbyGameMode::RearrangePlayers, 0.1f, false);
}

void ALobbyGameMode::CacheSpawnPoints()
{
	if (CachedSpawnPoints.Num() == 0)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbySpawnPoint::StaticClass(), FoundActors);
		for (AActor* Actor : FoundActors)
		{
			if (ALobbySpawnPoint* Point = Cast<ALobbySpawnPoint>(Actor))
			{
				CachedSpawnPoints.Add(Point);
			}
		}
		CachedSpawnPoints.Sort([](const ALobbySpawnPoint& A, const ALobbySpawnPoint& B) {
			return A.GetActorLocation().Y < B.GetActorLocation().Y;
		});
	}
}

void ALobbyGameMode::SpawnLobbyCharacter(APlayerController* NewPlayer)
{
	if (!NewPlayer || CachedSpawnPoints.Num() < 1) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 NumberOfPlayers = GetNumPlayers();
	if (LobbyPlayerPreviewClass && CachedSpawnPoints.Num() >= NumberOfPlayers)
	{
		ALobbySpawnPoint* ChosenPoint = CachedSpawnPoints[NumberOfPlayers - 1];
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Spawning player at point %d: %s"), NumberOfPlayers - 1, *ChosenPoint->GetName()));
		ALobbyPlayerPreview* NewPreview = GetWorld()->SpawnActor<ALobbyPlayerPreview>(
			LobbyPlayerPreviewClass,
			ChosenPoint->GetActorLocation(),
			ChosenPoint->GetActorRotation(),
			SpawnParams
		);

		if (ALobbyPlayerController* MyPc = Cast<ALobbyPlayerController>(NewPlayer))
		{
			MyPc->MyPreviewActor = NewPreview;
			if (ALobbyPlayerState* PS = MyPc->GetPlayerState<ALobbyPlayerState>())
			{
				PS->OnPlayerColorChanged.AddDynamic(NewPreview, &ALobbyPlayerPreview::SetPlayerColor);
				NewPreview->SetPlayerColor(PS->GetPlayerColor());
			} 
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyPlayerPreviewClass is not assigned in the GameMode!"));
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

void ALobbyGameMode::RearrangePlayers()
{
	int32 PlayerIndex = 0;
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get()))
		{
			if (CachedSpawnPoints.IsValidIndex(PlayerIndex))
			{
				ALobbySpawnPoint* NewPoint = CachedSpawnPoints[PlayerIndex];
				if (PC->MyPreviewActor)
				{
					PC->MyPreviewActor->SetActorLocation(NewPoint->GetActorLocation());
				}
			}
		}
		PlayerIndex++;
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

bool ALobbyGameMode::CanHandleNewPlayer()
{
	if (PlayerCount >= MaxPlayer)
	{
		// Translated: "Refus de connexion : lobby plein"
		UE_LOG(LogTemp, Warning, TEXT("Connection refused: lobby full"));
		return false;
	}
	return true;
}

void ALobbyGameMode::OnNewPlayerLogin(int32 PlayerId, const FString& PlayerName, bool bIsHost)
{
}

void ALobbyGameMode::CheckGameStart()
{
	ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>();
	if (LobbyGameState && LobbyGameState->AreAllPlayersReady())
	{
		StartGame();
	}
}

void ALobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(TEXT("/Game/Maps/Gym/CompleteGym?listen"));
	}
}