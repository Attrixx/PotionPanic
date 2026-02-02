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
#include "Kismet/GameplayStatics.h"

ALobbyGameMode::ALobbyGameMode()
{
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerCount++;
	TArray<AActor *> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbySpawnPoint::StaticClass(), FoundPoints);

	ALobbySpawnPoint *ChosenPoint = nullptr;

	for (AActor *Actor : FoundPoints)
	{
		ALobbySpawnPoint *TestPoint = Cast<ALobbySpawnPoint>(Actor);
		if (TestPoint && !TestPoint->bIsOccupied)
		{
			ChosenPoint = TestPoint;
			break;
		}
	}
	if (ChosenPoint)
	{
		ChosenPoint->bIsOccupied = true;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = NewPlayer;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ALobbyPlayerPreview *NewPreview = GetWorld()->SpawnActor<ALobbyPlayerPreview>(
				ALobbyPlayerPreview::StaticClass(),
				ChosenPoint->GetActorLocation(),
				ChosenPoint->GetActorRotation(),
				SpawnParams);

		if (ALobbyPlayerController *MyPc = Cast<ALobbyPlayerController>(NewPlayer))
		{
			MyPc->MyPreviewActor = NewPreview;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Pas de SpawnPoint libre trouv� !"));
	}

	if (ALobbyPlayerState* PlayerState = NewPlayer->GetPlayerState<ALobbyPlayerState>())
	{
		// Handle Couch Coop Player Naming
		FString PlayerName = PlayerState->GetPlayerName();
		FString BaseName = PlayerName;
		TArray<int32> UsedIndices;
		bool bFoundSiblings = false;

		for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* OtherPC = It->Get();
			if (OtherPC && OtherPC != NewPlayer)
			{
				bool bIsSameConnection = false;
				if (NewPlayer->GetNetConnection() != nullptr)
				{
					// Client: Check if sharing the same connection
					if (OtherPC->GetNetConnection() == NewPlayer->GetNetConnection())
					{
						bIsSameConnection = true;
					}
				}
				else
				{
					// Host/Local: Check if both are local controllers
					if (OtherPC->IsLocalController() && NewPlayer->IsLocalController())
					{
						bIsSameConnection = true;
					}
				}

				if (bIsSameConnection)
				{
					bFoundSiblings = true;
					if (APlayerState* OtherPS = OtherPC->GetPlayerState<APlayerState>())
					{
						FString OtherName = OtherPS->GetPlayerName();
						FString OtherBaseName = OtherName;
						int32 OtherIndex = 0;

						int32 OpenParenIdx;
						if (OtherName.FindLastChar('(', OpenParenIdx) && OtherName.EndsWith(TEXT(")")))
						{
							FString IndexStr = OtherName.Mid(OpenParenIdx + 1, OtherName.Len() - OpenParenIdx - 2);
							if (IndexStr.IsNumeric())
							{
								OtherIndex = FCString::Atoi(*IndexStr);
								OtherBaseName = OtherName.Left(OpenParenIdx);
							}
						}

						// Assume the BaseName is the one from the 'main' player (index 0 or just consistent)
						// We consistently update BaseName to match potential parents
						if (BaseName == PlayerName) // Only update if we haven't found a better candidate or to strictly match
						{
							BaseName = OtherBaseName;
						}
						
						UsedIndices.Add(OtherIndex);
					}
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
			
			PlayerName = FString::Printf(TEXT("%s(%d)"), *BaseName, NewIndex);
			PlayerState->SetPlayerName(PlayerName);
		}

		bool bIsHost = NewPlayer->HasAuthority() && NewPlayer->IsLocalController() && !bFoundSiblings;
		if (bIsHost)
		{
			bHostPlayerIdInitialized = true;
			HostPlayerId = PlayerState->GetPlayerId();
			PlayerState->Server_SetIsHost(true);
		}
		// Log UniqueNetId for debugging
		const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
		// Case No OnlineSubsystem, UniqueId may be invalid
		if (UniqueId.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("PostLogin: PlayerName: %s, PlayerId: %d, UniqueId: %s, PlayerController: %s"), *PlayerState->GetPlayerName(), PlayerState->GetPlayerId(), *UniqueId.GetV1()->ToDebugString(), *NewPlayer->GetName());
		}

		OnNewPlayerLogin(PlayerState->GetPlayerId(), PlayerState->GetPlayerName(), bIsHost);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (!Exiting->PlayerState) return;
	int32 PlayerId = Exiting->PlayerState->GetPlayerId();

	if (ALobbyPlayerController *LeavingPC = Cast<ALobbyPlayerController>(Exiting))
	{
		TArray<AActor *> FoundPoints;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbySpawnPoint::StaticClass(), FoundPoints);

		// SAVE SPAWNPOINT IN PC
		for (AActor *Actor : FoundPoints)
		{
			ALobbySpawnPoint *Point = Cast<ALobbySpawnPoint>(Actor);
			if (Point && FVector::DistSquared(Point->GetActorLocation(), LeavingPC->MyPreviewActor->GetActorLocation()) < 2500.0f)
			{

				Point->bIsOccupied = false;
				break;
			}
		}
		LeavingPC->MyPreviewActor->Destroy();
	}

	PlayerCount--;
	if (PlayerCount < 0)
		PlayerCount = 0;
	UE_LOG(LogTemp, Log, TEXT("Joueur parti"));
	Super::Logout(Exiting);
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Adress, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Adress, UniqueId, ErrorMessage);
	if (!CanHandleNewPlayer())
	{
		ErrorMessage = TEXT("Le lobby est plein ");
	}
}

void ALobbyGameMode::OnNewPlayerLogin(int32 PlayerId, const FString& PlayerName, bool bIsHost)
{
	
}

bool ALobbyGameMode::CanHandleNewPlayer()
{
	if (PlayerCount >= MaxPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Refus de connexion : lobby plein"));
		return false; 
	}
	return true;
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
		// TODO: Replace with actual map name
		World->ServerTravel(TEXT("/Game/Maps/Gym/CompleteGym?listen")); 
	}
}
