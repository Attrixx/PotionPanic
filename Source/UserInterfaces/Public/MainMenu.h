#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MainMenu.generated.h"

UCLASS()
class USERINTERFACES_API UMainMenu : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

protected:

	void NativeConstruct() override;

protected:

	UPROPERTY(meta = (BindWidget))
	class UButton* BT_Host;

	UPROPERTY(meta = (BindWidget))
	class UButton* BT_Join;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString LobbyMapURL;

private:

	TObjectPtr<class UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

protected:

	UFUNCTION()
	void OnHostClicked();
	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnJoinSessions(EOnJoinSessionCompleteResult::Type Result);
	void OnAcceptInvite(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

};
