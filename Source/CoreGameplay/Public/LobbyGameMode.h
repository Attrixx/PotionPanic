// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFrameWork/OnlineReplStructs.h"
#include "LobbyGameMode.generated.h"


UCLASS()
class COREGAMEPLAY_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALobbyGameMode();

protected:

	void BeginPlay() override;

public:
	virtual void PostLogin(APlayerController* NewPlayer) override; 
	virtual void Logout(AController* Exiting) override; 
	virtual void PreLogin(const FString& Options, const FString& Adress, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	bool CanHandleNewPlayer();

private:

	int32 MaxPlayer = 4; 
	int32 PlayerCount = 0;
	
};
