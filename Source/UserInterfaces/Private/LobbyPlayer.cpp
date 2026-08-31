#include "LobbyPlayer.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"

void ULobbyPlayer::UpdatePlayerInfo(int32 PlayerId, FString PlayerName, bool bIsHost, bool bIsCouchCoopPlayer, bool bIsReady, FColor PlayerColor)
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
	Overlay_HostCrown->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Overlay_IsReady->SetVisibility(bIsReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Text_PlayerName->SetText(FText::FromString(PlayerName));
	Text_PlayerName->SetColorAndOpacity(FSlateColor(PlayerColor));
}

void ULobbyPlayer::OnPlayerLeave()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}
