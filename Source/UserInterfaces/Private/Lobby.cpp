// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby.h"
#include "LobbyPlayer.h"
#include "LobbyGameState.h"
#include "LobbyPlayerState.h"

#include "Components/HorizontalBox.h"

void ULobby::NativeConstruct()
{
	Super::NativeConstruct();
	
	for (int i = 0; i < 4; ++i)
	{
		if (LobbyPlayerWidgetClass)
		{
			ULobbyPlayer* LobbyPlayerWidget = CreateWidget<ULobbyPlayer>(GetWorld(), LobbyPlayerWidgetClass);
			if (LobbyPlayerWidget && HBox_Players)
			{
				HBox_Players->AddChildToHorizontalBox(LobbyPlayerWidget);
			}
		}
	}

	if (ALobbyGameState* GameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		GameState->OnPlayerAdded.AddDynamic(this, &ULobby::HandlePlayerAdded);
		GameState->OnPlayerRemoved.AddDynamic(this, &ULobby::HandlePlayerRemoved);

		for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
		{
			if (ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(GameState->PlayerArray[i]))
			{
				LobbyPlayerState->OnPlayerInfoChanged.AddDynamic(this, &ULobby::UpdatePlayerWidgets);
			}
		}
	}
}

void ULobby::UpdatePlayerWidgets()
{
	if (!HBox_Players) return;

	if (ALobbyGameState* GameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
		{
			if (ALobbyPlayerState* State = Cast<ALobbyPlayerState>(GameState->PlayerArray[i]))
			{

				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Updating Player Widget for PlayerId: %d, PlayerName: %s"), State->GetPlayerId(), *State->GetPlayerName()));

				if (ULobbyPlayer* PlayerWidget = Cast<ULobbyPlayer>(HBox_Players->GetChildAt(i)))
				{
					PlayerWidget->UpdatePlayerInfo(
						State->GetPlayerId(),
						State->GetPlayerName(),
						State->IsHost(),
						State->IsCouchCoopPlayer(),
						State->IsReady(),
						State->GetPlayerColor()
					);
					PlayerWidget->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}

		for (int32 i = GameState->PlayerArray.Num(); i < HBox_Players->GetChildrenCount(); ++i)
		{
			if (ULobbyPlayer* PlayerWidget = Cast<ULobbyPlayer>(HBox_Players->GetChildAt(i)))
			{
				PlayerWidget->OnPlayerLeave();
			}
		}
	}
}
void ULobby::HandlePlayerAdded(APlayerState* PlayerState)
{
	if (ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState))
	{
		LobbyPlayerState->OnPlayerInfoChanged.AddDynamic(this, &ULobby::UpdatePlayerWidgets);
		UpdatePlayerWidgets();
	}
}

void ULobby::HandlePlayerRemoved(APlayerState* PlayerState)
{
	if (ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState))
	{
		LobbyPlayerState->OnPlayerInfoChanged.RemoveDynamic(this, &ULobby::UpdatePlayerWidgets);
		UpdatePlayerWidgets();
	}
}
