// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerPreferences.generated.h"

/**
 * 
 */
UCLASS()
class PLAYER_API UPlayerPreferences : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	// TODO: save keybinds...
};
