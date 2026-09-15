// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PotionPanicGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API UPotionPanicGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:

	virtual void Init() override;
	void StartGameInstance() override;

	void OnJoinSessions(EOnJoinSessionCompleteResult::Type Result);
	void OnAcceptInvite(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

private:

	TObjectPtr<class UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

};
