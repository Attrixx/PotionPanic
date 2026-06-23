#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerState.h"
#include "LobbyWidget.generated.h"

class UHorizontalBox;
class ULobbyPlayer;

UCLASS()
class USERINTERFACES_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	void NativeConstruct() override;

public:

	UFUNCTION()
	void UpdatePlayerWidgets();

	UFUNCTION()
	void HandlePlayerAdded(APlayerState* PlayerState);

	UFUNCTION()
	void HandlePlayerRemoved(APlayerState* PlayerState);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_Players;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<ULobbyPlayer> LobbyPlayerWidgetClass;
};
