#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayer.generated.h"

class UWidgetSwitcher;
class UOverlay;
class UTextBlock;

UCLASS()
class USERINTERFACES_API ULobbyPlayer : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdatePlayerInfo(int32 PlayerId, FString PlayerName, bool bIsHost, bool bIsCouchCoopPlayer, bool bIsReady, FColor PlayerColor);

	void OnPlayerLeave();

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_HostCrown;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_IsReady;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerName;
};
