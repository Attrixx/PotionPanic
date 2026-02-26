// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StartupGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API AStartupGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString LobbyMapURL;
	
};
