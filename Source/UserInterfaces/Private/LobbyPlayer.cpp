// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayer.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"

void ULobbyPlayer::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULobbyPlayer::UpdatePlayerInfo(int32 PlayerId, FString PlayerName, bool bIsHost, bool bIsCouchCoopPlayer, bool bIsReady, FColor PlayerColor)
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
	Overlay_HostCrown->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Overlay_IsReady->SetVisibility(bIsReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Text_PlayerName->SetText(FText::FromString(PlayerName));
}

void ULobbyPlayer::OnPlayerLeave()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}
